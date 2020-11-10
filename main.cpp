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

using Promotions = std::vector<std::unique_ptr<Promotion>>;

Price calculate_price(const Cart& cart) {
    Price price = 0;
    for (SKU sku : cart) {
        price += unit_prices.at(sku);
    }
    return price;
}

Price calculate_price(const Cart& c, const Promotion* promotion) {
    Cart cart = c;
    Price price = promotion->promote(cart);
    price += calculate_price(cart);
    return price;
}

Price calculate_price(const Cart& c, const Promotions& promotions) {
    Cart cart = c;
    Price price = 0;
    for (const auto& promotion : promotions) {
        price += promotion->promote(cart);
    }
    price += calculate_price(cart);
    return price;
}

int main() {
    {
        // Empty cart
        Cart cart("");
        assert(calculate_price(cart) == 0);
    }
    {
        // "ab" == "ba"
        Cart cart1("ab");
        Cart cart2("ba");
        assert(calculate_price(cart1) == calculate_price(cart2));
    }
    {
        // 2 identical SKUs
        Cart cart("aa");
        assert(calculate_price(cart) == 100);
    }
    {
        // Scenario A
        Cart cart("abc");
        assert(calculate_price(cart) == 100);
    }
    {
        // Test Individual promotion
        auto promotion = std::make_unique<Individual>(3, 'a', 130);
        {
            // Not enough SKUs for promotion
            Cart cart("aa");
            assert(calculate_price(cart, promotion.get()) == 100);
        }
        {
            // Exactly enough SKUs for 1 promotion
            Cart cart("aaa");
            assert(calculate_price(cart, promotion.get()) == 130);
        }
        {
            // Exactly enough SKUs for 2 promotions
            Cart cart("aaaaaa");
            assert(calculate_price(cart, promotion.get()) == 260);
        }
    }
    {
        // Test Combined promotion
        auto promotion = std::make_unique<Combined>('c', 'd', 30);
        {
            // Not enough SKUs for promotion
            Cart cart("c");
            assert(calculate_price(cart, promotion.get()) == 20);
        }
        {
            // Not enough SKUs for promotion
            Cart cart("bc");
            assert(calculate_price(cart, promotion.get()) == 50);
        }
        {
            // Exactly enough SKUs for 1 promotion
            Cart cart("cd");
            assert(calculate_price(cart, promotion.get()) == 30);
        }
        {
            // Exactly enough SKUs for 2 promotions
            Cart cart("ccdd");
            assert(calculate_price(cart, promotion.get()) == 60);
        }
        {
            // "cd" == "dc"
            Cart cart1("cd");
            Cart cart2("dc");
            assert(calculate_price(cart1, promotion.get()) == calculate_price(cart2, promotion.get()));
        }
    }
    {
        Promotions promotions;
        promotions.push_back(std::make_unique<Individual>(3, 'a', 130));
        promotions.push_back(std::make_unique<Individual>(2, 'b', 45));
        promotions.push_back(std::make_unique<Combined>('c', 'd', 30));
        {
            // Scenario B
            Cart cart("aaaaabbbbbc");
            assert(calculate_price(cart, promotions) == 370);
        }
        {
            // Scenario C
            Cart cart("aaabbbbbcd");
            assert(calculate_price(cart, promotions) == 280);
        }
    }
}
