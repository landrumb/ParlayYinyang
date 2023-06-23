#include "types.h"

using namespace parlay;

template <typename T> sequence<T> operator/(const sequence<T>& a, const double b) {
  return parlay::map(a, [=] (T v) {return (T) (v/b);}, 100);}


template <typename T> sequence<T> operator+(const sequence<T>& a, const slice<T*,T*>& b) {
  return parlay::tabulate(a.size(), [&] (size_t i) {return a[i] + b[i];}, 100);}


template <typename T> sequence<T> operator+(const sequence<T>& a, const sequence<T>& b) {
  if (a.size() == 0) return b;
  return parlay::tabulate(a.size(), [&] (size_t i) {return a[i] + b[i];}, 100);}


int main() {

    sequence<int> me(5,16);
    sequence<int> divved = me/3;
    std::cout << divved[0] << std::endl;

    sequence<int> me2(6,4);
    sequence<int> me3(6,5);
    slice<int*,int*> me3slice = make_slice(me3);
    sequence<int> added = me2 + me3slice;
    std::cout << added[0] << std::endl;
    
}

// Example
// template <typename T> sequence<T> operator/(const sequence<T>& a, const double b) {
//   return parlay::map(a, [=] (T v) {return v/b;}, 100);}


// int main() {

//     sequence<int> me(5,16);
//     sequence<int> divved = me/3;
//     std::cout << divved[0] << std::endl;
    
// }