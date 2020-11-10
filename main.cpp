#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

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

// Abstract base class
class Promotion {
public:
    ~Promotion() = default;

    Price promote(Cart& cart) const { return do_promote(cart); }

private:
    virtual Price do_promote(Cart& cart) const = 0;
};

// Class that implements individual promotion
class Individual : public Promotion {
public:
    explicit Individual(int n, SKU sku, Price price) : n_(n), sku_(sku), price_(price) {}

private:
    Price do_promote(Cart& cart) const override {
        Price price = 0;
        const std::string p(n_, sku_);
        std::string::size_type pos = cart.find(p);
        while (pos != std::string::npos) {
            cart.erase(pos, n_);
            pos = cart.find(p);
            price += price_;
        }
        return price;
    }

    int n_;
    SKU sku_;
    Price price_;
};

// Class that implements combined promotion
class Combined : public Promotion {
public:
    explicit Combined(SKU sku1, SKU sku2, Price price) : sku1_(sku1), sku2_(sku2), price_(price) {}

private:
    Price do_promote(Cart& cart) const override {
        Price price = 0;
        std::string::size_type pos1 = cart.find(sku1_);
        std::string::size_type pos2 = cart.find(sku2_);
        while (pos1 != std::string::npos && pos2 != std::string::npos) {
            // Erase SKUs from cart
            if (pos1 > pos2) {
                std::swap(pos1, pos2);
            }
            cart.erase(pos2, 1);
            cart.erase(pos1, 1);

            price += price_;

            pos1 = cart.find(sku1_);
            pos2 = cart.find(sku2_);
        }
        return price;
    }

    SKU sku1_;
    SKU sku2_;
    Price price_;
};

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
    {
        // Test Individual promotion
        auto promotion = std::make_unique<Individual>(3, 'a', 130);
        {
            // Not enough SKUs for promotion
            Cart cart("aa");
            Price price = promotion->promote(cart);
            assert(price == 0);
        }
        {
            // Exactly enough SKUs for 1 promotion
            Cart cart("aaa");
            Price price = promotion->promote(cart);
            assert(price == 130);
        }
        {
            // Exactly enough SKUs for 2 promotions
            Cart cart("aaaaaa");
            Price price = promotion->promote(cart);
            assert(price == 260);
        }
    }
    {
        // Test Combined promotion
        auto promotion = std::make_unique<Combined>('c', 'd', 30);
        {
            // Not enough SKUs for promotion
            Cart cart("c");
            Price price = promotion->promote(cart);
            assert(price == 0);
        }
        {
            // Not enough SKUs for promotion
            Cart cart("bc");
            Price price = promotion->promote(cart);
            assert(price == 0);
        }
        {
            // Exactly enough SKUs for 1 promotion
            Cart cart("cd");
            Price price = promotion->promote(cart);
            assert(price == 30);
        }
        {
            // Exactly enough SKUs for 2 promotions
            Cart cart("ccdd");
            Price price = promotion->promote(cart);
            assert(price == 60);
        }
        {
            // "cd" == "dc"
            Cart cart1("cd");
            Price price1 = promotion->promote(cart1);
            Cart cart2("dc");
            Price price2 = promotion->promote(cart2);
            assert(price1 == price2);
        }
    }
    {
        std::vector<std::unique_ptr<Promotion>> active_promotions;
        active_promotions.push_back(std::make_unique<Individual>(3, 'a', 130));
        active_promotions.push_back(std::make_unique<Individual>(2, 'b', 45));
        active_promotions.push_back(std::make_unique<Combined>('c', 'd', 30));
        {
            // Scenario B
            Cart cart("aaaaabbbbbc");
            Price price = 0;
            for (const auto& p : active_promotions) {
                price += p->promote(cart);
            }
            price += charge(cart);
            assert(price == 370);
        }
        {
            // Scenario C
            Cart cart("aaabbbbbcd");
            Price price = 0;
            for (const auto& p : active_promotions) {
                price += p->promote(cart);
            }
            price += charge(cart);
            assert(price == 280);
        }
    }
}
