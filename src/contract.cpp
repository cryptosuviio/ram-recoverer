/*
 * Copyright 2019-2020 Ville Sundell/CRYPTOSUVI OSK
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/name.hpp>
#include <eosio.system/eosio.system.hpp>
#include <eosio.token/eosio.token.hpp>

/* We can leave debug messages on, since that could help solving problems on Mainnet */
#define DEBUG(...) print(__FILE__, ":", __LINE__, " ", __VA_ARGS__, "\n");
#define SKIP_AND_CONTINUE(x) DEBUG(x, " Skipping ", account_iterator->account_name, "..."); add_skipped(account_iterator->account_name); account_iterator = account_list.erase(account_iterator); continue;

using namespace eosio;

class [[eosio::contract("contract")]] _contract : public eosio::contract {
   public:
      using eosio::contract::contract;

      struct [[eosio::table]] account {
         name account_name;
         auto primary_key() const { return account_name.value; }
      };

      typedef multi_index<"accounts"_n, account> accounts;
      typedef multi_index<"skipped"_n, account> skipped;

      void add_skipped(name account_name) {
         skipped skipped_list(get_self(), get_self().value);

         skipped_list.emplace(get_self(), [&](auto& a) {
            a.account_name = account_name;
         });
      }

      void add_internal(name account_name) {
         accounts account_list(get_self(), get_self().value);

         account_list.emplace(get_self(), [&](auto& a) {
            a.account_name = account_name;
         });
      }

      [[eosio::action]]
      void add(std::vector<name> account_names) {
         require_auth(get_self());

         for(auto& account_name : account_names) {
            add_internal(account_name);
            DEBUG("Added account: ", account_name);
         }
      }


      [[eosio::action]]
      void remove(std::vector<name> account_names) {
         require_auth(get_self());

         for(auto& account_name : account_names) {
            accounts account_list(get_self(), get_self().value);

            auto account_iterator = account_list.find(account_name.value);
            if(account_iterator != account_list.end()) {
               account_list.erase(account_iterator);
            }

            skipped skipped_list(get_self(), get_self().value);

            auto skipped_iterator = skipped_list.find(account_name.value);
            if(skipped_iterator != skipped_list.end()) {
               skipped_list.erase(skipped_iterator);
            }

            DEBUG("Removed account: ", account_name);
         }
      }

      [[eosio::action]]
      void sellram(uint8_t n) {
         int i;
         accounts account_list(get_self(), get_self().value);

         auto account_iterator = account_list.begin();

         for(i = 0; i < n && account_iterator != account_list.end(); i++) {
            DEBUG("Selling RAM from: ", account_iterator->account_name);

            eosiosystem::user_resources_table userres("eosio"_n, account_iterator->account_name.value);

            auto res_itr = userres.find(account_iterator->account_name.value);
            check(res_itr != userres.end(), "User has no resource entry");

            int64_t ram_to_sell = res_itr->ram_bytes - 3010;
            if(ram_to_sell <= 0) {
               SKIP_AND_CONTINUE("Not enough RAM to sell");
            }

            eosiosystem::rammarket rm ("eosio"_n, "eosio"_n.value);
            auto ram_itr = rm.find(symbol(symbol_code("RAMCORE"), 4).raw());
            check(ram_itr != rm.end(), "No RAM Market?");

            //auto sold_ram = eosiosystem::exchange_state::get_bancor_output(ram_itr->base.balance.amount, ram_itr->quote.balance.amount, ram_to_sell);
            //Calculating the RAM price using Bancor:
            auto sold_ram = int64_t( (ram_to_sell * ram_itr->quote.balance.amount) / (ram_itr->base.balance.amount + ram_to_sell) );
            if(sold_ram <= 0) {
               SKIP_AND_CONTINUE("RAM price too low");
            }

            auto fee = ( sold_ram + 199 ) / 200;

            eosiosystem::system_contract::sellram_action sellram("eosio"_n, {account_iterator->account_name, "active"_n});
            sellram.send(account_iterator->account_name, ram_to_sell);

            token::transfer_action transfer("eosio.token"_n, {account_iterator->account_name, "active"_n});
            auto amount_to_recover = asset(sold_ram - fee, symbol(symbol_code("TLOS"),4));
            if(amount_to_recover.amount <= 0) {
               SKIP_AND_CONTINUE("Transfer amount too small");
            }

            transfer.send(account_iterator->account_name, get_self(), amount_to_recover, "Recovering RAM per TBNOA: https://chainspector.io/dashboard/ratify-proposals/0");

            account_iterator = account_list.erase(account_iterator);
         }

         check(i > 0, "No accounts to sell RAM from");
      }

      [[eosio::action]]
      void retry(uint8_t n) {
         int i;
         skipped skipped_list(get_self(), get_self().value);
         accounts account_list(get_self(), get_self().value);

         auto account_iterator = account_list.begin();
         check(account_iterator == account_list.end(), "Accounts list must be empty first");

         auto skipped_iterator = skipped_list.begin();

         for(i = 0; i < n && skipped_iterator != skipped_list.end(); i++) {
            add_internal(skipped_iterator->account_name);
            skipped_iterator = skipped_list.erase(skipped_iterator);
         }

         check(i > 0, "No accounts copy from skipped accounts list");
      }
};
