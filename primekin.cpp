#include <iostream>
#include <cinttypes>
#include <cmath>
#include <chrono>
#include <bitset>

const uint64_t limit = 1E6; // find primes up to this
const uint64_t blob_size = 64; // corresponds to "uint64_t *blobs" in Sieve
const uint64_t blob_mask = blob_size-1; // for faster modulos
const uint64_t n_blobs = limit/blob_size / 2 + 1; // divide by 2 as we only store odds
const uint64_t blob_div = 1 + log2(blob_size); // divide by 2 and then blob_size
const uint64_t root = sqrt(limit);

const uint64_t ONE = 1; // 64 bit "1" for bit shifting >= 32 bits

class Sieve
{
    // std::bitset doesn't support openmp atomic operations
    // this is my own implementation of flip, reset and test
    uint64_t *blobs;
    public:
    Sieve() { blobs = new uint64_t[n_blobs](); } // "()" clears memory
    ~Sieve() { delete[] blobs; }
    void flip(uint64_t p);
    void reset(uint64_t p);
    bool test(uint64_t p);
};

inline void Sieve::flip(uint64_t p) {
    // flipping is the only thing done in parallel
    // the other functions do support atomic operations though
    // in my tests bitshifts are a bit faster than regular division
    #pragma omp atomic
    blobs[p >> blob_div] ^= ONE << ((p>>1)&blob_mask); // faster way to do p/2 % 64?
}

inline void Sieve::reset(uint64_t p) {
    blobs[p >> blob_div] &= ~(ONE << ((p>>1)&blob_mask));
}

inline bool Sieve::test(uint64_t p) {
    return (blobs[p >> blob_div] & (ONE << ((p>>1)&blob_mask))) != 0;
}

int main()
{
    std::cout << "Finding primes up to " << limit << std::endl;
    auto start = std::chrono::high_resolution_clock::now();

    Sieve *sieve = new Sieve;

    #pragma omp parallel
    {
        for (uint64_t x = 1; x <= root; x++) {
            uint64_t xx = x*x;
            uint64_t threexx = xx + xx + xx;

            // let the next thread execute the loop by itself, but in parallel
            // with the other threads
            #pragma omp single nowait
            for (uint64_t y = 1; y <= root; y++) {
                uint64_t n = threexx - y*y;
                if (x > y && n <= limit && n&1 && n%12 == 11)
                    sieve->flip(n);

                n = threexx + y*y;
                if (n <= limit && n&1 && n%12 == 7)
                    sieve->flip(n);

                n += xx;
                if (n <= limit && n&1 && (n%12 == 1 || n%12 == 5))
                    sieve->flip(n);
            }
        }
    }

    // remove multiples of the found primes
    for (uint64_t n = 5; n <= root; n+=2)
        if (sieve->test(n)) {
            uint64_t nn = n*n;
            for (uint64_t k = nn; k <= limit; k += nn<<1) // *2 to skip evens
                sieve->reset(k);
        }

    // print the primes
    //std::cout << 2 << std::endl << 3 << std::endl;
    uint64_t count = 2;
    for (uint64_t n = 5; n <= limit; n+=2)
        if (sieve->test(n)) {
            //std::cout << n << std::endl;
            count++;
        }

    delete sieve;

    auto end = std::chrono::high_resolution_clock::now();
    auto time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

    std::cout << "Found " << count << " primes under " << limit << " in " << time_spent << " ms" << std::endl;
    return 0;
}
