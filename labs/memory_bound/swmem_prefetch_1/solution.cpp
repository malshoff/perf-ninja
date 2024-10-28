#include "solution.hpp"

static int getSumOfDigits(int n) {
  int sum = 0;
  while (n != 0) {
    sum = sum + n % 10;
    n = n / 10;
  }
  return sum;
}

int solution(const hash_map_t *hash_map, const std::vector<int> &lookups) {
  int result = 0;
  const int prefetch_distance = 32;
  for (int i = 0; i + prefetch_distance < lookups.size(); ++i) {
    hash_map->prefetch(lookups[i + prefetch_distance]);
    if (hash_map->find(lookups[i]))
      result += getSumOfDigits(lookups[i]);
  }

  for (int i = lookups.size() - prefetch_distance; i < lookups.size(); ++i) {
    if (hash_map->find(lookups[i]))
      result += getSumOfDigits(lookups[i]);
  }

  return result;
}
