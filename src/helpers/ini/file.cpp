/*

This file is provided under the Creative Commons 0 License.
License: <https://creativecommons.org/publicdomain/zero/1.0/legalcode>
Summary: <https://creativecommons.org/publicdomain/zero/1.0/>

One-line summary: This file is public domain or the closest legal equivalent.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/
#include "./file.h"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <limits>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QString>
#include "../strings.h"

#include "./setting.h"

namespace {
   constexpr char c_iniComment       = ';';
   constexpr char c_iniCategoryStart = '[';
   constexpr char c_iniCategoryEnd   = ']';
   constexpr char c_iniKeyValueDelim = '=';

   int _ascii_casecmp(const char* a, const char* b) {
      if (!a || !b)
         return (a == b) ? 0 : (a ? 1 : -1);
      for (; *a && *b; ++a, ++b) {
         auto aa = (unsigned char)*a;
         auto bb = (unsigned char)*b;
         aa = (unsigned char)std::tolower(aa);
         bb = (unsigned char)std::tolower(bb);
         if (aa != bb)
            return int(aa) - int(bb);
      }
      return int((unsigned char)*a) - int((unsigned char)*b);
   }

   bool _path_is_json(const std::filesystem::path& path) {
      auto ext = path.extension().string();
      std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char c) {
         return (char)std::tolower(c);
      });
      return ext == ".json";
   }

   bool _json_value_to_bool(const QJsonValue& value, bool& out) {
      if (value.isBool()) {
         out = value.toBool();
         return true;
      }
      if (value.isDouble()) {
         out = value.toDouble() != 0.0;
         return true;
      }
      if (value.isString()) {
         auto string = value.toString().toStdString();
         if (cobb::string_says_false(string.c_str())) {
            out = false;
            return true;
         }
         float numeric = 0.0f;
         if (cobb::string_to_float(string.c_str(), numeric)) {
            out = numeric != 0.0f;
            return true;
         }
         return false;
      }
      return false;
   }

   void _apply_json_setting(cobb::ini::setting* current, const QJsonValue& value) {
      if (!current || value.isUndefined() || value.isNull())
         return;
      switch (current->type) {
         case cobb::ini::setting_type::boolean:
            {
               bool v = current->current.b;
               if (_json_value_to_bool(value, v))
                  current->current.b = v;
            }
            return;
         case cobb::ini::setting_type::float32:
            if (value.isDouble()) {
               current->current.f = (float)value.toDouble();
               return;
            }
            if (value.isString()) {
               float numeric = 0.0f;
               auto  string  = value.toString().toStdString();
               if (cobb::string_to_float(string.c_str(), numeric))
                  current->current.f = numeric;
            }
            return;
         case cobb::ini::setting_type::integer_signed:
            if (value.isDouble()) {
               auto numeric = value.toDouble();
               if (std::isfinite(numeric) && numeric >= (double)std::numeric_limits<int32_t>::min() && numeric <= (double)std::numeric_limits<int32_t>::max())
                  current->current.i = (int32_t)numeric;
               return;
            }
            if (value.isString()) {
               int32_t numeric = 0;
               auto    string  = value.toString().toStdString();
               if (cobb::string_to_int(string.c_str(), numeric, true))
                  current->current.i = numeric;
            }
            return;
         case cobb::ini::setting_type::integer_unsigned:
            if (value.isDouble()) {
               auto numeric = value.toDouble();
               if (std::isfinite(numeric) && numeric >= 0.0 && numeric <= (double)std::numeric_limits<uint32_t>::max())
                  current->current.u = (uint32_t)numeric;
               return;
            }
            if (value.isString()) {
               uint32_t numeric = 0;
               auto     string  = value.toString().toStdString();
               if (cobb::string_to_int(string.c_str(), numeric, true))
                  current->current.u = numeric;
            }
            return;
         case cobb::ini::setting_type::string:
            if (value.isString()) {
               current->currentStr = value.toString().toStdString();
               return;
            }
            if (value.isBool()) {
               current->currentStr = value.toBool() ? "TRUE" : "FALSE";
               return;
            }
            if (value.isDouble()) {
               current->currentStr = std::to_string(value.toDouble());
               return;
            }
            if (value.isArray() || value.isObject()) {
               QJsonDocument doc(value.isObject() ? QJsonDocument(value.toObject()) : QJsonDocument(value.toArray()));
               current->currentStr = doc.toJson(QJsonDocument::Compact).toStdString();
            }
            return;
      }
   }

   QJsonValue _setting_to_json_value(const cobb::ini::setting* setting) {
      if (!setting)
         return QJsonValue();
      switch (setting->type) {
         case cobb::ini::setting_type::boolean:
            return setting->current.b;
         case cobb::ini::setting_type::float32:
            return setting->current.f;
         case cobb::ini::setting_type::integer_signed:
            return setting->current.i;
         case cobb::ini::setting_type::integer_unsigned:
            return (qint64)setting->current.u;
         case cobb::ini::setting_type::string:
            return QString::fromUtf8(setting->currentStr.c_str());
      }
      return QJsonValue();
   }
}

namespace cobb::ini {
   void file::_pending_category::write(file* const manager, std::fstream& file) {
      if (this->name.empty())
         return;
      file.write(this->header.c_str(), this->header.size());
      {  // Write missing settings.
         try {
            const auto& list = manager->_get_category_contents(this->name);
            for (auto it = list.begin(); it != list.end(); ++it) {
               const setting* const setting = *it;
               if (std::find(this->found.begin(), this->found.end(), setting) == this->found.end()) {
                  std::string line = setting->name;
                  line += '=';
                  line += setting->to_string();
                  line += "\n"; // MSVC appears to sometimes treat '\n' in a const char* as "\r\n", automatically, without asking, so don't add a '\r'
                  file.write(line.c_str(), line.size());
               }
            }
         } catch (std::out_of_range) {};
      }
      file.write(this->body.c_str(), this->body.size());
   }

   file::file() {
   }
   file::file(std::filesystem::path filepath) {
      this->set_paths(filepath);
   }
   file::file(std::filesystem::path filepath, std::filesystem::path backup, std::filesystem::path working) {
      this->set_paths(filepath, backup, working);
   }

   void file::set_paths(std::filesystem::path filepath) {
      this->filePath = filepath;
      std::filesystem::path w = filepath;
      std::filesystem::path b = w;
      auto ext = filepath.extension().string();
      if (ext.empty())
         ext = ".settings";
      w.replace_extension(ext + ".tmp");
      b.replace_extension(ext + ".bak");
      this->backupFilePath  = b;
      this->workingFilePath = w;
   }
   void file::set_paths(std::filesystem::path filepath, std::filesystem::path backup, std::filesystem::path working) {
      this->filePath        = filepath;
      this->backupFilePath  = backup;
      this->workingFilePath = working;
   }
   
   file::setting_list& file::_get_category_contents(std::string category) {
      std::transform(category.begin(), category.end(), category.begin(), [](unsigned char c) {
         return (char)std::tolower(c);
      });
      return this->byCategory[category];
   }
   void file::_send_change_event(setting* s, setting_value_union oldValue, setting_value_union newValue) {
      auto copy = this->changeCallbacks; // in case a callback unregisters itself
      for (auto cb : copy)
         cb(s, oldValue, newValue);
   }
   setting* file::get_setting(const char* category, const char* name) const {
      if (!category || !name)
         return nullptr;
      for (auto it = this->settings.begin(); it != this->settings.end(); ++it) {
         auto& setting = *it;
         if (!_ascii_casecmp(setting->category, category) && !_ascii_casecmp(setting->name, name))
            return setting;
      }
      return nullptr;
   }
   setting* file::get_setting(std::string& category, std::string& name) const {
      return this->get_setting(category.c_str(), name.c_str());
   }
   void file::get_categories(std::vector<std::string>& out) const {
      out.reserve(this->byCategory.size());
      for (auto it = this->byCategory.begin(); it != this->byCategory.end(); ++it)
         out.push_back(it->first);
   }
   void file::insert_setting(setting* setting) {
      this->settings.push_back(setting);
      //
      std::string category = setting->category;
      std::transform(category.begin(), category.end(), category.begin(), [](unsigned char c) {
         return (char)std::tolower(c);
      });
      this->byCategory[category].push_back(setting);
   }
   //
   void file::load() {
      std::ifstream file;
      file.open(this->filePath, std::ios_base::in | std::ios_base::binary);
      if (!file) {
         printf("Unable to load settings file for reading. Calling cobb::ini::file::save to generate a default one.\n");
         this->save(); // generate a new settings file.
         return;
      }
      if (_path_is_json(this->filePath)) {
         std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
         file.close();
         QJsonParseError parseError;
         auto doc = QJsonDocument::fromJson(QByteArray(contents.data(), (qsizetype)contents.size()), &parseError);
         if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
            printf("Unable to parse settings JSON file (%s). Regenerating defaults.\n", parseError.errorString().toUtf8().constData());
            this->save();
            return;
         }
         auto root = doc.object();
         for (auto category = root.begin(); category != root.end(); ++category) {
            if (!category.value().isObject())
               continue;
            auto categoryName = category.key().toStdString();
            auto settings     = category.value().toObject();
            for (auto entry = settings.begin(); entry != settings.end(); ++entry) {
               auto key = entry.key().toStdString();
               if (auto* current = this->get_setting(categoryName.c_str(), key.c_str()))
                  _apply_json_setting(current, entry.value());
            }
         }
         this->abandon_pending_changes();
         return;
      }
      std::string category = "";
      std::string key      = "";
      while (!file.bad() && !file.eof()) {
         char buffer[1024];
         file.getline(buffer, sizeof(buffer));
         buffer[1023] = '\0';
         //
         bool     foundAny   = false; // found anything that isn't whitespace?
         bool     isCategory = false;
         setting* current    = nullptr;
         size_t   i = 0;
         char     c = buffer[0];
         #pragma region Category headers and INI setting names
         if (!c)
            continue;
         do {
            if (!foundAny) {
               if (c == ' ' || c == '\t')
                  continue;
               if (c == c_iniComment) // lines starting with semicolons are comments
                  break;
               foundAny = true;
               if (c == c_iniCategoryStart) {
                  isCategory = true;
                  category   = "";
                  continue;
               }
               key = "";
            }
            if (isCategory) {
               if (c == c_iniCategoryEnd)
                  break;
               category += c;
               continue;
            }
            //
            // handling for setting names:
            //
            if (!current) {
               if (c == c_iniKeyValueDelim) {
                  current = this->get_setting(category, key);
                  i++;
                  break;
               }
               key += c;
            }
         } while (++i < sizeof(buffer) && (c = buffer[i]));
         #pragma endregion
         //
         // Code from here on out assumes that (i) will not be modified -- it will always refer 
         // either to the end of the line or to the first character after the '='.
         //
         #pragma region INI setting value
         if (current) { // handling for setting values
            if (current->type != setting_type::string) {  // Allow comments on the same line as a setting (unless the setting value is a string)
               size_t j = i;
               do {
                  if (buffer[j] == '\0')
                     break;
                  if (buffer[j] == ';') {
                     buffer[j] = '\0';
                     break;
                  }
               } while (++j < sizeof(buffer));
            }
            switch (current->type) {
               case setting_type::boolean:
                  {
                     if (cobb::string_says_false(buffer + i)) {
                        current->current.b = false;
                        break;
                     }
                     //
                     // treat numbers that compute to zero as "false:"
                     //
                     float value;
                     bool  final = true;
                     if (cobb::string_to_float(buffer + i, value)) {
                        final = (value != 0.0);
                     }
                     current->current.b = final;
                  }
                  break;
               case setting_type::float32:
                  {
                     float value;
                     if (cobb::string_to_float(buffer + i, value))
                        current->current.f = value;
                  }
                  break;
               case setting_type::integer_signed:
                  {
                     int32_t value;
                     if (cobb::string_to_int(buffer + i, value))
                        current->current.i = value;
                  }
                  break;
               case setting_type::integer_unsigned:
                  {
                     uint32_t value;
                     if (cobb::string_to_int(buffer + i, value))
                        current->current.u = value;
                  }
                  break;
               case setting_type::string:
                  current->currentStr.clear();
                  do {
                     char c = buffer[i];
                     if (c == '\0')
                        break;
                     current->currentStr += c;
                  } while (++i < sizeof(buffer));
                  break;
            }
         }
         #pragma endregion
      }
      file.close();
      this->abandon_pending_changes();
   };
   void file::save() {
      for (auto& setting : this->settings) {
         setting->commit_pending_changes();
      }
      if (_path_is_json(this->filePath)) {
         QJsonObject root;
         for (auto* setting : this->settings) {
            const QString category = QString::fromUtf8(setting->category);
            const QString key      = QString::fromUtf8(setting->name);
            auto categoryObject    = root.value(category).toObject();
            categoryObject.insert(key, _setting_to_json_value(setting));
            root.insert(category, categoryObject);
         }
         auto json = QJsonDocument(root).toJson(QJsonDocument::Indented);
         std::fstream oFile;
         oFile.open(this->workingFilePath, std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
         if (!oFile) {
            printf("Unable to open working settings file for writing.\n");
            return;
         }
         oFile.write(json.constData(), json.size());
         oFile.close();
         std::error_code ec;
         if (std::filesystem::exists(this->filePath, ec)) {
            ec.clear();
            std::filesystem::copy_file(
               this->filePath,
               this->backupFilePath,
               std::filesystem::copy_options::overwrite_existing,
               ec
            );
         }
         ec.clear();
         std::filesystem::rename(this->workingFilePath, this->filePath, ec);
         if (ec) {
            ec.clear();
            std::filesystem::copy_file(
               this->workingFilePath,
               this->filePath,
               std::filesystem::copy_options::overwrite_existing,
               ec
            );
            if (!ec)
               std::filesystem::remove(this->workingFilePath, ec);
         }
         return;
      }
      //
      std::fstream oFile;
      std::fstream iFile;
      oFile.open(this->workingFilePath, std::ios_base::out | std::ios_base::trunc);
      if (!oFile) {
         printf("Unable to open working INI file for writing.\n");
         return;
      }
      //
      _pending_category current;
      std::vector<std::string> missingCategories;
      this->get_categories(missingCategories);
      iFile.open(this->filePath, std::ios_base::in);
      if (iFile) {
         //
         // If the INI file already exists, read its contents: we want to preserve whitespace, comments, 
         // the order of settings, and so on. This loop reads the original file and then writes them (with 
         // any needed changes to INI setting values) to the working file.
         //
         bool beforeCategories = true;
         while (!iFile.bad() && !iFile.eof()) {
            std::string line;
            getline(iFile, line);
            line += '\n'; // add the terminator, since it (but not '\r') will be omitted from getline's result.
            size_t i    = 0;
            size_t size = line.size();
            bool   foundAny   = false; // found any non-whitespace chars
            bool   isCategory = false;
            bool   isValue    = false;
            std::string token;
            do {
               unsigned char c = line[i];
               if (!c)
                  break;
               if (c == c_iniComment) { // comment
                  if (isValue) {
                     size_t j = i; // we want to preserve any whitespace that followed the original value
                     do {
                        char d = line[--j];
                        if (!isspace(d))
                           break;
                     } while (j);
                     current.body += (line.c_str() + j + 1); // at this point, j will include one non-whitespace character BEFORE the trailing whitespace
                     break;
                  }
                  current.body += line;
                  break;
               }
               if (c == '\r' || c == '\n') { // end of line; make sure we get the whole terminator (we need this to catch the case of a setting value with no trailing comment)
                  current.body += (line.c_str() + i);
                  break;
               }
               if (!foundAny) {
                  if (c == c_iniCategoryStart) {
                     isCategory = true;
                     beforeCategories = false;
                     continue;
                  }
                  if (isspace(c))
                     continue;
                  foundAny = true;
               }
               if (isCategory) {
                  if (c == c_iniCategoryEnd) {
                     current.write(this, oFile);
                     current = _pending_category(token, line);
                     {
                        std::transform(token.begin(), token.end(), token.begin(), [](unsigned char d) {
                           return (char)std::tolower(d);
                        });
                        missingCategories.erase(std::remove(missingCategories.begin(), missingCategories.end(), token), missingCategories.end());
                     }
                     break;
                  }
                  token += c;
               } else {
                  if (c == c_iniKeyValueDelim) {
                     current.body.append(line, 0, i + 1);
                     auto ptr = this->get_setting(current.name.c_str(), token.c_str());
                     if (!ptr) {
                        current.body.append(line, i);
                        break;
                     }
                     current.body += ptr->to_string();
                     current.found.push_back(ptr);
                     isValue = true;
                     continue;
                  }
                  token += c;
               }
            } while (++i < size);
            if (beforeCategories) { // content before all categories
               oFile.write(line.c_str(), size);
            }
         }
         iFile.close();
      }
      current.write(this, oFile); // write the last category, if necessary
      {  // Write missing categories.
         //
         // Write any setting categories that weren't present in the existing file. (If there was 
         // no existing file, then this writes all categories, creating the file from scratch.)
         //
         for (auto& cat : missingCategories) {
            oFile.put('\n'); // MSVC treats std::fstream.put('\n') as .write("\r\n", 2) automatically, without asking, so don't put('\r')
            std::string header;
            cobb::sprintf(header, "[%s]\n", cat.c_str());
            current = _pending_category(cat, header);
            current.write(this, oFile);
         }
      }
      oFile.close();
      std::error_code ec;
      if (std::filesystem::exists(this->filePath, ec)) {
         ec.clear();
         std::filesystem::copy_file(
            this->filePath,
            this->backupFilePath,
            std::filesystem::copy_options::overwrite_existing,
            ec
         );
      }
      ec.clear();
      std::filesystem::rename(this->workingFilePath, this->filePath, ec);
      if (ec) {
         ec.clear();
         std::filesystem::copy_file(
            this->workingFilePath,
            this->filePath,
            std::filesystem::copy_options::overwrite_existing,
            ec
         );
         if (!ec) {
            std::filesystem::remove(this->workingFilePath, ec);
         }
      }
   }
   //
   void file::abandon_pending_changes() {
      for (auto& setting : this->settings)
         setting->abandon_pending_changes();
   }
   //
   void file::register_for_changes(change_callback cb) noexcept {
      if (!cb)
         return;
      auto& list = this->changeCallbacks;
      if (std::find(list.begin(), list.end(), cb) != list.end())
         return;
      list.push_back(cb);
   }
   void file::unregister_for_changes(change_callback cb) noexcept {
      if (!cb)
         return;
      auto& list = this->changeCallbacks;
      list.erase(std::remove(list.begin(), list.end(), cb));
   }
}
