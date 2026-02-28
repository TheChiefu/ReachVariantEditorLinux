#include "variables_and_scopes.h"
#include "opcode_arg_types/variables/all_core.h"

namespace Megalo {
   int8_t VariableScopeWhichValue::as_integer() const noexcept {
      if (!this->owner)
         return -1;
      return this->owner->index_of(*this);
   }
   int8_t VariableScopeWhichValueList::index_of(const VariableScopeWhichValue& v) const noexcept {
      size_t size = this->values.size();
      for (size_t i = 0; i < size; ++i)
         if (this->values[i] == &v)
            return i;
      return -1;
   }
   //
   namespace variable_which_values {
      using flags = VariableScopeWhichValue::flag;
      //
      namespace object {
          VariableScopeWhichValue no_object  = VariableScopeWhichValue("no_object", flags::is_none);
          VariableScopeWhichValue global_0   = VariableScopeWhichValue("global.object[0]");
          VariableScopeWhichValue global_1   = VariableScopeWhichValue("global.object[1]");
          VariableScopeWhichValue global_2   = VariableScopeWhichValue("global.object[2]");
          VariableScopeWhichValue global_3   = VariableScopeWhichValue("global.object[3]");
          VariableScopeWhichValue global_4   = VariableScopeWhichValue("global.object[4]");
          VariableScopeWhichValue global_5   = VariableScopeWhichValue("global.object[5]");
          VariableScopeWhichValue global_6   = VariableScopeWhichValue("global.object[6]");
          VariableScopeWhichValue global_7   = VariableScopeWhichValue("global.object[7]");
          VariableScopeWhichValue global_8   = VariableScopeWhichValue("global.object[8]");
          VariableScopeWhichValue global_9   = VariableScopeWhichValue("global.object[9]");
          VariableScopeWhichValue global_10  = VariableScopeWhichValue("global.object[10]");
          VariableScopeWhichValue global_11  = VariableScopeWhichValue("global.object[11]");
          VariableScopeWhichValue global_12  = VariableScopeWhichValue("global.object[12]");
          VariableScopeWhichValue global_13  = VariableScopeWhichValue("global.object[13]");
          VariableScopeWhichValue global_14  = VariableScopeWhichValue("global.object[14]");
          VariableScopeWhichValue global_15  = VariableScopeWhichValue("global.object[15]");
          VariableScopeWhichValue current    = VariableScopeWhichValue("current_object", flags::is_read_only | flags::is_transient);
          VariableScopeWhichValue hud_target = VariableScopeWhichValue("hud_target_object", flags::is_read_only);
          VariableScopeWhichValue killed     = VariableScopeWhichValue("killed_object", flags::is_read_only | flags::is_transient);
          VariableScopeWhichValue killer     = VariableScopeWhichValue("killer_object", flags::is_read_only | flags::is_transient);
          VariableScopeWhichValue unknown_21 = VariableScopeWhichValue("unknown_object_21", flags::is_read_only); // non-networked, too
          VariableScopeWhichValue temporary_0 = VariableScopeWhichValue("temporaries.object[0]", flags::is_transient);
          VariableScopeWhichValue temporary_1 = VariableScopeWhichValue("temporaries.object[1]", flags::is_transient);
          VariableScopeWhichValue temporary_2 = VariableScopeWhichValue("temporaries.object[2]", flags::is_transient);
          VariableScopeWhichValue temporary_3 = VariableScopeWhichValue("temporaries.object[3]", flags::is_transient);
          VariableScopeWhichValue temporary_4 = VariableScopeWhichValue("temporaries.object[4]", flags::is_transient);
          VariableScopeWhichValue temporary_5 = VariableScopeWhichValue("temporaries.object[5]", flags::is_transient);
          VariableScopeWhichValue temporary_6 = VariableScopeWhichValue("temporaries.object[6]", flags::is_transient);
          VariableScopeWhichValue temporary_7 = VariableScopeWhichValue("temporaries.object[7]", flags::is_transient);
         //
          const VariableScopeWhichValueList list = VariableScopeWhichValueList({
            &no_object,
            &global_0,
            &global_1,
            &global_2,
            &global_3,
            &global_4,
            &global_5,
            &global_6,
            &global_7,
            &global_8,
            &global_9,
            &global_10,
            &global_11,
            &global_12,
            &global_13,
            &global_14,
            &global_15,
            &current,
            &hud_target,
            &killed,
            &killer,
            &unknown_21,
            &temporary_0,
            &temporary_1,
            &temporary_2,
            &temporary_3,
            &temporary_4,
            &temporary_5,
            &temporary_6,
            &temporary_7,
         });
      }
      namespace player {
          VariableScopeWhichValue no_player  = VariableScopeWhichValue("no_player", flags::is_none);
          VariableScopeWhichValue player_0   = VariableScopeWhichValue("player[0]", flags::is_read_only);
          VariableScopeWhichValue player_1   = VariableScopeWhichValue("player[1]", flags::is_read_only);
          VariableScopeWhichValue player_2   = VariableScopeWhichValue("player[2]", flags::is_read_only);
          VariableScopeWhichValue player_3   = VariableScopeWhichValue("player[3]", flags::is_read_only);
          VariableScopeWhichValue player_4   = VariableScopeWhichValue("player[4]", flags::is_read_only);
          VariableScopeWhichValue player_5   = VariableScopeWhichValue("player[5]", flags::is_read_only);
          VariableScopeWhichValue player_6   = VariableScopeWhichValue("player[6]", flags::is_read_only);
          VariableScopeWhichValue player_7   = VariableScopeWhichValue("player[7]", flags::is_read_only);
          VariableScopeWhichValue player_8   = VariableScopeWhichValue("player[8]", flags::is_read_only);
          VariableScopeWhichValue player_9   = VariableScopeWhichValue("player[9]", flags::is_read_only);
          VariableScopeWhichValue player_10  = VariableScopeWhichValue("player[10]", flags::is_read_only);
          VariableScopeWhichValue player_11  = VariableScopeWhichValue("player[11]", flags::is_read_only);
          VariableScopeWhichValue player_12  = VariableScopeWhichValue("player[12]", flags::is_read_only);
          VariableScopeWhichValue player_13  = VariableScopeWhichValue("player[13]", flags::is_read_only);
          VariableScopeWhichValue player_14  = VariableScopeWhichValue("player[14]", flags::is_read_only);
          VariableScopeWhichValue player_15  = VariableScopeWhichValue("player[15]", flags::is_read_only);
          VariableScopeWhichValue global_0   = VariableScopeWhichValue("global.player[0]");
          VariableScopeWhichValue global_1   = VariableScopeWhichValue("global.player[1]");
          VariableScopeWhichValue global_2   = VariableScopeWhichValue("global.player[2]");
          VariableScopeWhichValue global_3   = VariableScopeWhichValue("global.player[3]");
          VariableScopeWhichValue global_4   = VariableScopeWhichValue("global.player[4]");
          VariableScopeWhichValue global_5   = VariableScopeWhichValue("global.player[5]");
          VariableScopeWhichValue global_6   = VariableScopeWhichValue("global.player[6]");
          VariableScopeWhichValue global_7   = VariableScopeWhichValue("global.player[7]");
          VariableScopeWhichValue current    = VariableScopeWhichValue("current_player", flags::is_read_only | flags::is_transient);
          VariableScopeWhichValue hud        = VariableScopeWhichValue("hud_player", flags::is_read_only);
          VariableScopeWhichValue hud_target = VariableScopeWhichValue("hud_target_player", flags::is_read_only);
          VariableScopeWhichValue killer     = VariableScopeWhichValue("killer_player", flags::is_read_only | flags::is_transient);
          VariableScopeWhichValue temporary_0 = VariableScopeWhichValue("temporaries.player[0]", flags::is_transient);
          VariableScopeWhichValue temporary_1 = VariableScopeWhichValue("temporaries.player[1]", flags::is_transient);
          VariableScopeWhichValue temporary_2 = VariableScopeWhichValue("temporaries.player[2]", flags::is_transient);
         //
          const VariableScopeWhichValueList list = VariableScopeWhichValueList({
            &no_player,
            &player_0,
            &player_1,
            &player_2,
            &player_3,
            &player_4,
            &player_5,
            &player_6,
            &player_7,
            &player_8,
            &player_9,
            &player_10,
            &player_11,
            &player_12,
            &player_13,
            &player_14,
            &player_15,
            &global_0,
            &global_1,
            &global_2,
            &global_3,
            &global_4,
            &global_5,
            &global_6,
            &global_7,
            &current,
            &hud, // local_player
            &hud_target, // target_player
            &killer,
            &temporary_0,
            &temporary_1,
            &temporary_2,
         });
      }
      namespace team {
          VariableScopeWhichValue no_team  = VariableScopeWhichValue("no_team", flags::is_none);
          VariableScopeWhichValue team_0   = VariableScopeWhichValue("team[0]", flags::is_read_only);
          VariableScopeWhichValue team_1   = VariableScopeWhichValue("team[1]", flags::is_read_only);
          VariableScopeWhichValue team_2   = VariableScopeWhichValue("team[2]", flags::is_read_only);
          VariableScopeWhichValue team_3   = VariableScopeWhichValue("team[3]", flags::is_read_only);
          VariableScopeWhichValue team_4   = VariableScopeWhichValue("team[4]", flags::is_read_only);
          VariableScopeWhichValue team_5   = VariableScopeWhichValue("team[5]", flags::is_read_only);
          VariableScopeWhichValue team_6   = VariableScopeWhichValue("team[6]", flags::is_read_only);
          VariableScopeWhichValue team_7   = VariableScopeWhichValue("team[7]", flags::is_read_only);
          VariableScopeWhichValue neutral_team = VariableScopeWhichValue("neutral_team", flags::is_read_only);
          VariableScopeWhichValue global_0 = VariableScopeWhichValue("global.team[0]");
          VariableScopeWhichValue global_1 = VariableScopeWhichValue("global.team[1]");
          VariableScopeWhichValue global_2 = VariableScopeWhichValue("global.team[2]");
          VariableScopeWhichValue global_3 = VariableScopeWhichValue("global.team[3]");
          VariableScopeWhichValue global_4 = VariableScopeWhichValue("global.team[4]");
          VariableScopeWhichValue global_5 = VariableScopeWhichValue("global.team[5]");
          VariableScopeWhichValue global_6 = VariableScopeWhichValue("global.team[6]");
          VariableScopeWhichValue global_7 = VariableScopeWhichValue("global.team[7]");
          VariableScopeWhichValue current  = VariableScopeWhichValue("current_team", flags::is_read_only | flags::is_transient);
          VariableScopeWhichValue hud_player_owner_team        = VariableScopeWhichValue("hud_player_team", flags::is_read_only);
          VariableScopeWhichValue hud_target_player_owner_team = VariableScopeWhichValue("hud_target_team", flags::is_read_only);
          VariableScopeWhichValue temporary_0 = VariableScopeWhichValue("temporaries.team[0]", flags::is_transient);
          VariableScopeWhichValue temporary_1 = VariableScopeWhichValue("temporaries.team[1]", flags::is_transient);
          VariableScopeWhichValue temporary_2 = VariableScopeWhichValue("temporaries.team[2]", flags::is_transient);
          VariableScopeWhichValue temporary_3 = VariableScopeWhichValue("temporaries.team[3]", flags::is_transient);
          VariableScopeWhichValue temporary_4 = VariableScopeWhichValue("temporaries.team[4]", flags::is_transient);
          VariableScopeWhichValue temporary_5 = VariableScopeWhichValue("temporaries.team[5]", flags::is_transient);
         //
          const VariableScopeWhichValueList list = VariableScopeWhichValueList({
            &no_team,
            &team_0, // defenders
            &team_1, // attackers
            &team_2,
            &team_3,
            &team_4,
            &team_5,
            &team_6,
            &team_7,
            &neutral_team,
            &global_0,
            &global_1,
            &global_2,
            &global_3,
            &global_4,
            &global_5,
            &global_6,
            &global_7,
            &current,
            &hud_player_owner_team, // local_team
            &hud_target_player_owner_team, // target_team: the team designator for `current_target_player` if it exists or `current_target_object` otherwise, per 343i
            &temporary_0,
            &temporary_1,
            &temporary_2,
            &temporary_3,
            &temporary_4,
            &temporary_5,
         });
      }
   }
    const VariableScopeWhichValueList megalo_scope_does_not_have_specifier = VariableScopeWhichValueList({});

    const VariableScope MegaloVariableScopeGlobal = VariableScope(megalo_scope_does_not_have_specifier, 12, 8, 8, 8, 16);
    const VariableScope MegaloVariableScopePlayer = VariableScope(variable_which_values::player::list, 8, 4, 4, 4, 4);
    const VariableScope MegaloVariableScopeObject = VariableScope(variable_which_values::object::list, 8, 4, 2, 4, 4);
    const VariableScope MegaloVariableScopeTeam   = VariableScope(variable_which_values::team::list,   8, 4, 4, 4, 6);
    const VariableScope MegaloVariableScopeTemporary = VariableScope(megalo_scope_does_not_have_specifier, 10, 0, 6, 3, 8);

   extern const VariableScope& getScopeObjectForConstant(variable_scope s) noexcept {
      switch (s) {
         case variable_scope::global:
            return MegaloVariableScopeGlobal;
         case variable_scope::player:
            return MegaloVariableScopePlayer;
         case variable_scope::object:
            return MegaloVariableScopeObject;
         case variable_scope::team:
            return MegaloVariableScopeTeam;
         case variable_scope::temporary:
            return MegaloVariableScopeTemporary;
      }
      assert(false && "Unknown variable scope!");
      #if defined(_MSC_VER)
         __assume(0); // suppress "not all paths return a value" by telling MSVC this is unreachable
      #else
         __builtin_unreachable();
      #endif
   }
   extern const VariableScope* getScopeObjectForConstant(variable_type s) noexcept {
      switch (s) {
         case variable_type::player:
            return &MegaloVariableScopePlayer;
         case variable_type::object:
            return &MegaloVariableScopeObject;
         case variable_type::team:
            return &MegaloVariableScopeTeam;
      }
      return nullptr;
   }
   extern variable_scope getScopeConstantForObject(const VariableScope& s) noexcept {
      if (&s == &MegaloVariableScopeGlobal)
         return variable_scope::global;
      if (&s == &MegaloVariableScopePlayer)
         return variable_scope::player;
      if (&s == &MegaloVariableScopeObject)
         return variable_scope::object;
      if (&s == &MegaloVariableScopeTeam)
         return variable_scope::team;
      if (&s == &MegaloVariableScopeTemporary)
         return variable_scope::temporary;
      return variable_scope::not_a_scope;
   }
   extern variable_scope getVariableScopeForTypeinfo(const OpcodeArgTypeinfo* t) noexcept {
      if (!t)
         return variable_scope::global;
      if (t == &OpcodeArgValueObject::typeinfo)
         return variable_scope::object;
      if (t == &OpcodeArgValuePlayer::typeinfo)
         return variable_scope::player;
      if (t == &OpcodeArgValueTeam::typeinfo)
         return variable_scope::team;
      return variable_scope::not_a_scope;
   }
   extern variable_type getVariableTypeForTypeinfo(const OpcodeArgTypeinfo* t) noexcept {
      if (t == &OpcodeArgValueScalar::typeinfo)
         return variable_type::scalar;
      if (t == &OpcodeArgValueObject::typeinfo)
         return variable_type::object;
      if (t == &OpcodeArgValuePlayer::typeinfo)
         return variable_type::player;
      if (t == &OpcodeArgValueTeam::typeinfo)
         return variable_type::team;
      if (t == &OpcodeArgValueTimer::typeinfo)
         return variable_type::timer;
      return variable_type::not_a_variable;
   }
   extern const VariableScope* getScopeObjectForTypeinfo(const OpcodeArgTypeinfo* ti) noexcept {
      if (!ti)
         return &MegaloVariableScopeGlobal;
      if (ti == &OpcodeArgValueObject::typeinfo)
         return &MegaloVariableScopeObject;
      if (ti == &OpcodeArgValuePlayer::typeinfo)
         return &MegaloVariableScopePlayer;
      if (ti == &OpcodeArgValueTeam::typeinfo)
         return &MegaloVariableScopeTeam;
      return nullptr;
   }
}
