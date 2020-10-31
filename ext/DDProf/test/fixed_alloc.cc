#include <catch2/catch.hpp>
#include <ddprof/fixed_alloc.hh>
#include <ddprof/string_table.hh>
#include <deque>

template <class T, std::size_t BufferSize = 8192>
using FixedDeque = std::deque<T, ddprof::fixed_alloc<T, BufferSize>>;

TEST_CASE("fixed_alloc basic w/deque", "[fixed_alloc]") {
    FixedDeque<int>::allocator_type::arena_type arena;
    {
        FixedDeque<int> deque{arena};

        bool threw = false;
        int i = 0;
        try {
            while (i < arena.size()) {
                deque.push_back(i++);
            }
        } catch (std::bad_alloc &bad_alloc) {
            threw = true;
        }

        REQUIRE(threw);
    }
    arena.reset();
}