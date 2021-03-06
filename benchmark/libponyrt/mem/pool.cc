#include <benchmark/benchmark.h>
#include <platform.h>
#include <mem/pool.h>

#define LARGE_ALLOC ponyint_pool_adjust_size(POOL_MAX + 1)

/// When we mmap, pull at least this many bytes.
#ifdef PLATFORM_IS_ILP32
#define POOL_MMAP (16 * 1024 * 1024) // 16 MB
#else
#define POOL_MMAP (128 * 1024 * 1024) // 128 MB
#endif

typedef char block_t[32];

class PoolBench: public ::benchmark::Fixture
{ 
  protected:
    virtual void SetUp(const ::benchmark::State& st);
    virtual void TearDown(const ::benchmark::State& st);
};

void PoolBench::SetUp(const ::benchmark::State& st)
{
  (void)st;
}

void PoolBench::TearDown(const ::benchmark::State& st)
{
  (void)st;
}

BENCHMARK_DEFINE_F(PoolBench, pool_index)(benchmark::State& st) {
  while (st.KeepRunning()) {
    ponyint_pool_index(st.range(0));
  }
  st.SetItemsProcessed(st.iterations());
}

BENCHMARK_REGISTER_F(PoolBench, pool_index)->RangeMultiplier(3)->Ranges({{1, 1024<<10}});

BENCHMARK_DEFINE_F(PoolBench, POOL_ALLOC$)(benchmark::State& st) {
  while (st.KeepRunning()) {
    void* p = POOL_ALLOC(block_t);
    st.PauseTiming();
    POOL_FREE(block_t, p);
    st.ResumeTiming();
  }
  st.SetItemsProcessed(st.iterations());
}

BENCHMARK_REGISTER_F(PoolBench, POOL_ALLOC$);

BENCHMARK_DEFINE_F(PoolBench, POOL_ALLOC_multiple$_)(benchmark::State& st) {
  int num_allocs = st.range(0);
  void** p = (void**)alloca(sizeof(void *) * num_allocs);
  while (st.KeepRunning()) {
    for(int i = 0; i < num_allocs; i++)
      p[i] = POOL_ALLOC(block_t);
    st.PauseTiming();
    for(int i = num_allocs - 1; i >= 0; i--)
      POOL_FREE(block_t, p[i]);
    st.ResumeTiming();
  }
  st.SetItemsProcessed(st.iterations()*num_allocs);
}

BENCHMARK_REGISTER_F(PoolBench, POOL_ALLOC_multiple$_)->Arg(1<<10);

BENCHMARK_DEFINE_F(PoolBench, POOL_FREE$)(benchmark::State& st) {
  while (st.KeepRunning()) {
    st.PauseTiming();
    void* p = POOL_ALLOC(block_t);
    st.ResumeTiming();
    POOL_FREE(block_t, p);
  }
  st.SetItemsProcessed(st.iterations());
}

BENCHMARK_REGISTER_F(PoolBench, POOL_FREE$);

BENCHMARK_DEFINE_F(PoolBench, POOL_ALLOC_FREE)(benchmark::State& st) {
  while (st.KeepRunning()) {
    void* p = POOL_ALLOC(block_t);
    POOL_FREE(block_t, p);
  }
  st.SetItemsProcessed(st.iterations());
}

BENCHMARK_REGISTER_F(PoolBench, POOL_ALLOC_FREE);

BENCHMARK_DEFINE_F(PoolBench, POOL_FREE_multiple$_)(benchmark::State& st) {
  int num_allocs = st.range(0);
  void** p = (void**)alloca(sizeof(void *) * num_allocs);
  while (st.KeepRunning()) {
    st.PauseTiming();
    for(int i = 0; i < num_allocs; i++)
      p[i] = POOL_ALLOC(block_t);
    st.ResumeTiming();
    for(int i = num_allocs - 1; i >= 0; i--)
      POOL_FREE(block_t, p[i]);
  }
  st.SetItemsProcessed(st.iterations()*num_allocs);
}

BENCHMARK_REGISTER_F(PoolBench, POOL_FREE_multiple$_)->Arg(1<<10);

BENCHMARK_DEFINE_F(PoolBench, POOL_ALLOC_FREE_multiple)(benchmark::State& st) {
  int num_allocs = st.range(0);
  void** p = (void**)alloca(sizeof(void *) * num_allocs);
  while (st.KeepRunning()) {
    for(int i = 0; i < num_allocs; i++)
      p[i] = POOL_ALLOC(block_t);
    for(int i = num_allocs - 1; i >= 0; i--)
      POOL_FREE(block_t, p[i]);
  }
  st.SetItemsProcessed(st.iterations()*num_allocs);
}

BENCHMARK_REGISTER_F(PoolBench, POOL_ALLOC_FREE_multiple)->Arg(1<<10);

BENCHMARK_DEFINE_F(PoolBench, pool_alloc_size$)(benchmark::State& st) {
  while (st.KeepRunning()) {
    void* p = ponyint_pool_alloc_size(LARGE_ALLOC);
    st.PauseTiming();
    ponyint_pool_free_size(LARGE_ALLOC, p);
    st.ResumeTiming();
  }
  st.SetItemsProcessed(st.iterations());
}

BENCHMARK_REGISTER_F(PoolBench, pool_alloc_size$);

BENCHMARK_DEFINE_F(PoolBench, pool_alloc_size_multiple$_)(benchmark::State& st) {
  size_t num_allocs = st.range(0);
  void** p = (void**)alloca(sizeof(void *) * num_allocs);
  while (st.KeepRunning()) {
    for(size_t i = 0; i < num_allocs; i++)
      p[i] = ponyint_pool_alloc_size(LARGE_ALLOC);
    st.PauseTiming();
    for(size_t i = num_allocs; i > 0; i--)
      ponyint_pool_free_size(LARGE_ALLOC, p[i-1]);
    st.ResumeTiming();
  }
  st.SetItemsProcessed(st.iterations()*num_allocs);
}

BENCHMARK_REGISTER_F(PoolBench, pool_alloc_size_multiple$_)->Arg((int)(POOL_MMAP/LARGE_ALLOC));

BENCHMARK_DEFINE_F(PoolBench, pool_free_size$)(benchmark::State& st) {
  while (st.KeepRunning()) {
    st.PauseTiming();
    void* p = ponyint_pool_alloc_size(LARGE_ALLOC);
    st.ResumeTiming();
    ponyint_pool_free_size(LARGE_ALLOC, p);
  }
  st.SetItemsProcessed(st.iterations());
}

BENCHMARK_REGISTER_F(PoolBench, pool_free_size$);

BENCHMARK_DEFINE_F(PoolBench, pool_alloc_free_size)(benchmark::State& st) {
  while (st.KeepRunning()) {
    void* p = ponyint_pool_alloc_size(LARGE_ALLOC);
    ponyint_pool_free_size(LARGE_ALLOC, p);
  }
  st.SetItemsProcessed(st.iterations());
}

BENCHMARK_REGISTER_F(PoolBench, pool_alloc_free_size);

BENCHMARK_DEFINE_F(PoolBench, pool_free_size_multiple$_)(benchmark::State& st) {
  int num_allocs = st.range(0);
  void** p = (void**)alloca(sizeof(void *) * num_allocs);
  while (st.KeepRunning()) {
    st.PauseTiming();
    for(int i = 0; i < num_allocs; i++)
      p[i] = ponyint_pool_alloc_size(LARGE_ALLOC);
    st.ResumeTiming();
    for(int i = num_allocs - 1; i >= 0; i--)
      ponyint_pool_free_size(LARGE_ALLOC, p[i]);
  }
  st.SetItemsProcessed(st.iterations()*num_allocs);
}

BENCHMARK_REGISTER_F(PoolBench, pool_free_size_multiple$_)->Arg((int)(POOL_MMAP/LARGE_ALLOC));

BENCHMARK_DEFINE_F(PoolBench, pool_alloc_free_size_multiple)(benchmark::State& st) {
  int num_allocs = st.range(0);
  void** p = (void**)alloca(sizeof(void *) * num_allocs);
  while (st.KeepRunning()) {
    for(int i = 0; i < num_allocs; i++)
      p[i] = ponyint_pool_alloc_size(LARGE_ALLOC);
    for(int i = num_allocs - 1; i >= 0; i--)
      ponyint_pool_free_size(LARGE_ALLOC, p[i]);
  }
  st.SetItemsProcessed(st.iterations()*num_allocs);
}

BENCHMARK_REGISTER_F(PoolBench, pool_alloc_free_size_multiple)->Arg((int)(POOL_MMAP/LARGE_ALLOC));
