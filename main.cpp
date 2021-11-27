#include <memory>
#include <vector>
#include <iostream>
#include <optional>
#include <version>
#include <chrono>
#include <sys/time.h>
#include <string>
#include <strstream>
#include <fmt/chrono.h>
#include <fmt/ostream.h>
#include <algorithm>
#include <cstdint>
#include <benchmark/benchmark.h>

using namespace std;
/* On enter buf points to the end of the buffer, e.g. where the least
 * significant digit of the input number will be printed. Returns pointer to
 * where the most significant digit were printed, including zero padding.
 * Does NOT add zero at the end of buffer, this is responsibility of the caller.
 */
template<typename T, const unsigned base = 10, const unsigned width = 1>
static inline
  char* ritoa(T u, char *buf)
{
  static_assert(std::is_unsigned_v<T>, "signed types are not supported");
  static_assert(base <= 16, "extend character map below to support higher bases");
  unsigned digits = 0;
  while (u) {
    *--buf = "0123456789abcdef"[u % base];
    u /= base;
    digits++;
  }
  while (digits++ < width)
    *--buf = '0';
  return buf;
}

struct shard_id_t {
  int8_t id;

  shard_id_t() : id(0) {}
  explicit shard_id_t(int8_t _id) : id(_id) {}

  operator int8_t() const { return id; }

  const  static shard_id_t NO_SHARD;

};

const shard_id_t shard_id_t::NO_SHARD{};


  // placement seed (a hash value)
typedef uint32_t ps_t;

struct pg_t {
  uint64_t m_pool;
  uint32_t m_seed;

  pg_t() : m_pool(0), m_seed(0) {}
  pg_t(ps_t seed, uint64_t pool) :
                                   m_pool(pool), m_seed(seed) {}
  static const uint8_t calc_name_buf_size = 36;  // max length for max values len("18446744073709551615.ffffffff") + future suffix len("_head") + '\0'
  char* calc_name(char *buf, const char *suffix_backwords) const;

};

struct spg_t {
  pg_t pgid;
  shard_id_t shard;
  spg_t() : shard(shard_id_t::NO_SHARD) {}
  spg_t(pg_t pgid, shard_id_t shard) : pgid(pgid), shard(shard) {}
  explicit spg_t(pg_t pgid) : pgid(pgid), shard(shard_id_t::NO_SHARD) {}
  static const uint8_t calc_name_buf_size = pg_t::calc_name_buf_size + 4; // 36 + len('s') + len("255");
  char *calc_name(char *buf, const char *suffix_backwords) const;
  bool is_no_shard() const {
    return shard == shard_id_t::NO_SHARD;
  }

};

char* pg_t::calc_name(char *buf, const char *suffix_backwords) const
{
  while (*suffix_backwords)
    *--buf = *suffix_backwords++;

  buf = ritoa<uint32_t, 16>(m_seed, buf);

  *--buf = '.';

  return  ritoa<uint64_t, 10>(m_pool, buf);
}

char *spg_t::calc_name(char *buf, const char *suffix_backwords) const
{
  while (*suffix_backwords)
    *--buf = *suffix_backwords++;

  if (!is_no_shard()) {
    buf = ritoa<uint8_t, 10>((uint8_t)shard.id, buf);
    *--buf = 's';
  }

  return pgid.calc_name(buf, "");
}

// -----------------------------------------------------------------------------


template <>
struct fmt::formatter<spg_t> {
  constexpr auto parse(format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const spg_t& s, FormatContext& ctx)
  {
    if (s.is_no_shard()) {
      return fmt::format_to(ctx.out(), "{}.{:x}", s.pgid.m_pool, s.pgid.m_seed);
    } else {
      return fmt::format_to(ctx.out(), "{}.{:x}s{}", s.pgid.m_pool, s.pgid.m_seed, s.shard.id);
    }
  }
};



// -----------------------------------------------------------------------------

string calc_spgt_old(const spg_t& m_pg_id)
{
  // create the formatted ID string
  char buf[spg_t::calc_name_buf_size];
  buf[spg_t::calc_name_buf_size - 1] =
    '\0';  // RRR copied - and I don't like the orig
  return string{m_pg_id.calc_name(buf + spg_t::calc_name_buf_size - 1, "")};
}

static void ExistingCalcName(benchmark::State& state) {
  // Code before the loop is not measured
  pg_t pg1{0x7654, 23452345};
  spg_t p1{pg1, shard_id_t{3}};
  auto test1 = calc_spgt_old(p1);
  std::cout << __func__ << " => " << test1 << "\n";

  for (auto _ : state) {
    auto ret = calc_spgt_old(p1);
  }
}
BENCHMARK(ExistingCalcName);

// -----------------------------------------------------------------------------

string calc_tmpl_fmt(const spg_t& m_pg_id)
{
  return fmt::format("{}", m_pg_id);
}

static void TmplFmtCalcName(benchmark::State& state) {
  // Code before the loop is not measured
  pg_t pg1{0x7654, 23452345};
  spg_t p1{pg1, shard_id_t{3}};
  auto test1 = calc_tmpl_fmt(p1);
  std::cout << __func__ << " => " << test1 << "\n";

  for (auto _ : state) {
    auto ret = calc_tmpl_fmt(p1);
  }
}
BENCHMARK(TmplFmtCalcName);

// -----------------------------------------------------------------------------

string direct_fmt(const spg_t& s)
{
  if (s.is_no_shard()) {
    return fmt::format("{}.{:x}", s.pgid.m_pool, s.pgid.m_seed);
  } else {
    return fmt::format("{}.{:x}s{}", s.pgid.m_pool, s.pgid.m_seed, s.shard.id);
  }
}

static void DirectFmtCalcName(benchmark::State& state) {
  // Code before the loop is not measured
  pg_t pg1{0x7654, 23452345};
  spg_t p1{pg1, shard_id_t{3}};
  auto test1 = direct_fmt(p1);
  std::cout << __func__ << " => " << test1 << "\n";

  for (auto _ : state) {
    auto ret = direct_fmt(p1);
  }
}
BENCHMARK(DirectFmtCalcName);




BENCHMARK_MAIN();


//int main(int ac, const char** av)
//{
//}
