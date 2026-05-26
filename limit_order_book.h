#pragma once

#include "order.h"
#include "memory_pool.h"

#include <cstdint>

class LOB{
private:
    struct PriceLevel{
        Order* head = nullptr;
        Order* tail = nullptr;
    };
    // MemoryPool<Order> *memory;
    const static uint32_t total_prices = 100032;
    const static uint32_t size1 = total_prices / 64;
    const static uint32_t size2 = size1 / 64;

    uint64_t bit1_bids[size1] = {0};
    uint64_t bit2_bids[size2] = {0};
    uint64_t bit1_asks[size1] = {0};
    uint64_t bit2_asks[size2] = {0};

    PriceLevel bids[total_prices];
    PriceLevel asks[total_prices];

    void add_bid_price(uint32_t pp){
        bit1_bids[pp>>6] |= 1ull <<(pp&63);
        
        pp >>= 6;
        bit2_bids[pp>>6] |= 1ull <<(pp&63);
    }
    void add_ask_price(uint32_t pp){
        bit1_asks[pp>>6] |= 1ull <<(pp&63);
        
        pp >>= 6;
        bit2_asks[pp>>6] |= 1ull <<(pp&63);
    }
    void remove_bid_price(uint32_t pp){
        uint64_t MX = ~0;
        bit1_bids[pp>>6] &= (MX ^ 1ull <<(pp&63));

        pp >>= 6;
        bool flag = bit1_bids[pp]==0;
        if (flag){
            bit2_bids[pp>>6] &= (MX ^ 1ull <<(pp&63));
        }
    }
    void remove_ask_price(uint32_t pp){
        uint64_t MX = ~0;
        bit1_asks[pp>>6] &= (MX ^ 1ull <<(pp&63));

        pp >>= 6;
        bool flag = bit1_asks[pp]==0;
        if (flag){
            bit2_asks[pp>>6] &= (MX ^ 1ull <<(pp&63));
        }
    }

    void add_ask(Order* ptr){
        int pp = ptr->price;
        ptr->prev = nullptr;
        if(asks[pp].head == nullptr){
            add_ask_price(pp);
            asks[pp].head = ptr;
            asks[pp].tail = ptr;
            ptr->next = nullptr;
        }
        
        else{
            ptr->next = asks[pp].head;
            asks[pp].head->prev = ptr;
            asks[pp].head = ptr;
        }
    }
    void add_bid(Order* ptr){
        int pp = ptr->price;
        ptr->prev = nullptr;
        if(bids[pp].head == nullptr){
            add_bid_price(pp);
            bids[pp].head = ptr;
            bids[pp].tail = ptr;
            ptr->next = nullptr;
        }
        
        else{
            ptr->next = bids[pp].head;
            bids[pp].head->prev = ptr;
            bids[pp].head = ptr;
        }
    }

    void remove_ask(int pp){
        if (asks[pp].head == nullptr)
            return;
        // Order* ptr = asks[pp].tail;

        if(asks[pp].tail->prev == nullptr){
            asks[pp].head = nullptr;
            remove_ask_price(pp);
        }
        else
            asks[pp].tail->prev->next = nullptr;
        
        asks[pp].tail = asks[pp].tail->prev;
        // memory->add(ptr);
    }
    void remove_bid(int pp){
        if (bids[pp].head == nullptr)
            return;
        // Order* ptr = bids[pp].tail;

        if(bids[pp].tail->prev == nullptr){
            bids[pp].head = nullptr;
            remove_bid_price(pp);
        }
        else
            bids[pp].tail->prev->next = nullptr;
        
        bids[pp].tail = bids[pp].tail->prev;
        // memory->add(ptr);
    }

    uint32_t get_bid(){
        for(int i=size2-1; i>=0; i--){
            if(bit2_bids[i]==0) continue;
            int bit2 = 63 - __builtin_clzll(bit2_bids[i]);
            int j = i*64 + bit2;
            int bit1 = 63 - __builtin_clzll(bit1_bids[j]);
            return j*64 + bit1;
        }
        return 0;
    }
    uint32_t get_ask(){
        for(uint32_t i=0; i<size2; i++){
            if(bit2_asks[i]==0) continue;
            int bit2 = __builtin_ctzll(bit2_asks[i]);
            int j = i*64 + bit2;
            int bit1 = __builtin_ctzll(bit1_asks[j]);
            return j*64 + bit1;
        }
        return total_prices;
    }

public:
    // LOB(MemoryPool<Order> *mpool){
    //     memory = mpool;
    // }

    void process(Order* ptr){
        if(ptr->side == OrderSide::SELL){
            uint32_t bid = get_bid();
            while(bid >= ptr->price and bids[bid].tail != nullptr){
                int q = std::min(ptr->quantity, bids[bid].tail->quantity);

                ptr->quantity -= q;
                bids[bid].tail->quantity -= q;

                if (bids[bid].tail->quantity == 0)
                    remove_bid(bid);
                if (ptr->quantity == 0){
                    // memory->add(ptr);
                    break;
                }
                bid = get_bid();
            }
            if (ptr->type == OrderType::LIMIT && ptr->quantity > 0)
                add_ask(ptr);
        }

        else{
            uint32_t ask = get_ask();
            while(ask <= ptr->price and asks[ask].tail != nullptr){
                int q = std::min(ptr->quantity, asks[ask].tail->quantity);

                ptr->quantity -= q;
                asks[ask].tail->quantity -= q;

                if (asks[ask].tail->quantity == 0)
                    remove_ask(ask);
                if (ptr->quantity == 0){
                    // memory->add(ptr);
                    break;
                }
                ask = get_ask();
            }
            if (ptr->type == OrderType::LIMIT && ptr->quantity > 0)
                add_bid(ptr);

        }
    }
};