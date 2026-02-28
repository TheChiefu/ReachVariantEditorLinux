#include "ini.h"
#include <QApplication>

namespace ReachINI {
   extern cobb::qt::ini::file& getForQt() {
      static cobb::qt::ini::file instance = cobb::qt::ini::file();
      return instance;
   }
   extern cobb::ini::file& get() {
      auto casted = dynamic_cast<cobb::ini::file*>(&(getForQt()));
      assert(casted != nullptr && "cobb::qt::ini::file should be able to cast to cobb::ini::file");
      return *casted;
   }

   // INI SETTING DEFINITIONS.
   //
   // The macro used here intentionally differs from the one used in the H file. Keep the namespaces 
   // synchronized between the two files.
   //
   #define REACHTOOL_MAKE_INI_SETTING(name, category, value)  cobb::ini::setting name = cobb::ini::setting(get, #name, category, value);
   namespace CodeEditor {
      REACHTOOL_MAKE_INI_SETTING(bOverrideBackColor,  "CodeEditor", false);
      REACHTOOL_MAKE_INI_SETTING(bOverrideTextColor,  "CodeEditor", false);
      REACHTOOL_MAKE_INI_SETTING(bOverrideLineNumberBackColor,    "CodeEditor", false);
      REACHTOOL_MAKE_INI_SETTING(bOverrideLineNumberTextColor,    "CodeEditor", false);
      REACHTOOL_MAKE_INI_SETTING(bOverrideLineNumberCurrentColor, "CodeEditor", false);
      REACHTOOL_MAKE_INI_SETTING(sBackColor,          "CodeEditor", "rgb(255, 255, 255)");
      REACHTOOL_MAKE_INI_SETTING(sTextColor,          "CodeEditor", "rgb(  0,   0,   0)");
      REACHTOOL_MAKE_INI_SETTING(sLineNumberBackColor,    "CodeEditor", "rgb( 41,  48,  64)");
      REACHTOOL_MAKE_INI_SETTING(sLineNumberTextColor,    "CodeEditor", "rgb(158, 171, 194)");
      REACHTOOL_MAKE_INI_SETTING(sLineNumberCurrentColor, "CodeEditor", "rgb(235, 241, 255)");
      REACHTOOL_MAKE_INI_SETTING(sFontFamily,         "CodeEditor", "Courier New");
      REACHTOOL_MAKE_INI_SETTING(sFormatCommentLine,  "CodeEditor", "rgb(106, 153,  85) italic");
      REACHTOOL_MAKE_INI_SETTING(sFormatCommentBlock, "CodeEditor", "rgb(106, 153,  85) italic");
      REACHTOOL_MAKE_INI_SETTING(sFormatKeyword,      "CodeEditor", "rgb(197, 134, 192) bold");
      REACHTOOL_MAKE_INI_SETTING(sFormatSubkeyword,   "CodeEditor", "rgb( 78, 201, 176) bold");
      REACHTOOL_MAKE_INI_SETTING(sFormatBoolean,      "CodeEditor", "rgb( 86, 156, 214) bold");
      REACHTOOL_MAKE_INI_SETTING(sFormatConstant,     "CodeEditor", "rgb(220, 220, 170)");
      REACHTOOL_MAKE_INI_SETTING(sFormatNamespace,    "CodeEditor", "rgb(156, 220, 254)");
      REACHTOOL_MAKE_INI_SETTING(sFormatEnumValue,    "CodeEditor", "rgb(181, 206, 168)");
      REACHTOOL_MAKE_INI_SETTING(sFormatNumber,       "CodeEditor", "rgb(181, 206, 168)");
      REACHTOOL_MAKE_INI_SETTING(sFormatOperator,     "CodeEditor", "rgb(212, 212, 212) bold");
      REACHTOOL_MAKE_INI_SETTING(sFormatMember,       "CodeEditor", "rgb(156, 220, 254)");
      REACHTOOL_MAKE_INI_SETTING(sFormatCall,         "CodeEditor", "rgb(220, 220, 170)");
      REACHTOOL_MAKE_INI_SETTING(sFormatStringSimple, "CodeEditor", "rgb(206, 145, 120)");
      REACHTOOL_MAKE_INI_SETTING(sFormatStringBlock,  "CodeEditor", "rgb(206, 145, 120)");
   }
   namespace Compiler {
      REACHTOOL_MAKE_INI_SETTING(bInlineIfs, "Compiler", true);
   }
   namespace DefaultLoadPath {
      REACHTOOL_MAKE_INI_SETTING(uPathType,   "DefaultLoadPath", (uint32_t)DefaultPathType::current_working_directory);
      REACHTOOL_MAKE_INI_SETTING(sCustomPath, "DefaultLoadPath", "");
   }
   namespace DefaultSavePath {
      REACHTOOL_MAKE_INI_SETTING(uPathType,                 "DefaultSavePath",(uint32_t)DefaultPathType::path_of_open_file);
      REACHTOOL_MAKE_INI_SETTING(bExcludeMCCBuiltInFolders, "DefaultSavePath", true);
      REACHTOOL_MAKE_INI_SETTING(sCustomPath,               "DefaultSavePath", "");
   }
   namespace Editing {
      REACHTOOL_MAKE_INI_SETTING(bHideFirefightNoOps, "Editing", true);
   }
   namespace UIWindowTitle {
      REACHTOOL_MAKE_INI_SETTING(bShowFullPath,     "UIWindowTitle", true);
      REACHTOOL_MAKE_INI_SETTING(bShowVariantTitle, "UIWindowTitle", true);
      REACHTOOL_MAKE_INI_SETTING(sTheme, "UIWindowTitle", "themes/default.qss");
   }
   #undef REACHTOOL_MAKE_INI_SETTING
}
