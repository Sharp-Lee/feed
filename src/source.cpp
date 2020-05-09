#include <source.hpp>

ACTION source::feed(uint64_t pairid, double price0, double price1)
{
   require_auth(get_self());

   check(price0 > 0 && price1 > 0, "price invalid");

   uint64_t price0_fixed = price0 * 10000l;
   uint64_t price1_fixed = price1 * 10000l;

   auto itr = _markets.find(pairid);

   check(itr != _markets.end(), "pair not found");

   auto last_sec = itr->last_update.sec_since_epoch();

   check(last_sec < current_time_point().sec_since_epoch(), "update too frequently");

   const time_point_sec now{current_time_point().sec_since_epoch()};

   uint64_t timeElapsed = 1;

   // means has history price
   if (last_sec > 0)
   {
      timeElapsed = current_time_point().sec_since_epoch() - itr->last_update.sec_since_epoch();
   }

   _markets.modify(itr, get_self(), [&](auto &s) {
      check(s.price0_cumulative_last < (s.price0_cumulative_last + price0_fixed * timeElapsed), "price0 cumulative overflow");
      check(s.price1_cumulative_last < (s.price1_cumulative_last + price1_fixed * timeElapsed), "price1 cumulative overflow");

      s.price0_cumulative_last += price0_fixed * timeElapsed;
      s.price1_cumulative_last += price1_fixed * timeElapsed;
      s.last_update = now;
   });
}

ACTION source::create(name contract0, name contract1, symbol sym0, symbol sym1)
{
   require_auth(get_self());

   _markets.emplace(get_self(), [&](auto &s) {
      s.mid = _markets.available_primary_key();
      s.contract0 = contract0;
      s.contract1 = contract1;
      s.sym0 = sym0;
      s.sym1 = sym1;
   });
}