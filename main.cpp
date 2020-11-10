#include <cassert>
#include <string>
#include <unordered_map>

using Cart = std::string;
using SKU = char;
using Price = int;  // Very basic data type but very easy to work with!

const std::unordered_map<SKU, Price> unit_prices = {{'a', 50}, {'b', 30}, {'c', 20}, {'d', 15}};

// Calculates price for all SKUs left in the cart after all promotions have beed performed
Price charge(Cart& cart) {
    Price price = 0;
    for (char c : cart) {
        price += unit_prices.at(c);
    }
    return price;
}

int main() {
    {
        // Empty cart
        Cart cart("");
        assert(charge(cart) == 0);
    }
    {
        // "ab" == "ba"
        Cart cart1("ab");
        Cart cart2("ba");
        assert(charge(cart1) == charge(cart2));
    }
    {
        // 2 identical SKUs
        Cart cart("aa");
        assert(charge(cart) == 100);
    }
    {
        // Scenario A
        Cart cart("abc");
        assert(charge(cart) == 100);
    }
}
