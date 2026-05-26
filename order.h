#pragma once
#include <cstdint>

enum class OrderType : uint8_t{
    LIMIT = 0,
    MARKET = 1
};
enum class OrderSide : uint8_t{
    SELL = 0,
    BUY = 1
};

struct Order{
    uint64_t id;
    uint32_t quantity;
    uint32_t price;
    OrderSide side;  // Buy or Sell
    Order* prev;
    Order* next;
    OrderType type;  // Market or Limit
};