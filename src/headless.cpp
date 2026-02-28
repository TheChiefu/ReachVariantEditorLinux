#include "./headless.h"
#include <cassert>
#include <cstdio>
#include <QDataStream>
#include <QFile>
#include <QSaveFile>
#include "./cli.h"
#include "./editor_state.h"
#include "./game_variants/base.h"
#include "./game_variants/io_process.h"
#include "./game_variants/warnings.h"
#include "./game_variants/components/megalo/compiler/compiler.h"
#include "./game_variants/components/megalo/decompiler/decompiler.h"

namespace rvt::headless {
   extern bool load_game_variant(const std::filesystem::path& path) {
      auto& editor  = ReachEditorState::get();
      auto* variant = new GameVariant;
      {
         auto    w_path = path.wstring();
         QString q_path = QString::fromStdWString(w_path);
         QFile   file(q_path);
         if (!file.open(QIODevice::ReadOnly)) {
            auto error = file.errorString().toStdString();
            std::fprintf(stderr, "Failed to open the game variant: %s\n", error.c_str());
            return false;
         }
         auto buffer = file.readAll();
         bool success;
         if (q_path.endsWith(".mglo", Qt::CaseInsensitive)) {
            success = variant->read_mglo(buffer.data(), buffer.size());
         } else {
            success = variant->read(buffer.data(), buffer.size());
         }
         if (!success) {
            auto error = file.errorString().toStdString();
            std::fprintf(stderr, "Failed to open the game variant: %s\n", error.c_str());
            return false;
         }
         {
            auto& log = GameEngineVariantLoadWarningLog::get();
            if (!log.warnings.empty()) {
               std::fputs("The following warnings were encountered:\n\n", stdout);
               for (auto& warning : log.warnings) {
                  auto text = warning.toStdString();
                  std::fprintf(stdout, " - %s\n", text.c_str());
               }
            }
         }

         editor.takeVariant(variant, w_path.c_str());
         return true;
      }
   }

   extern bool recompile(const command_line_params& params) {
      QString code;
      if (params.megalo_source.empty()) {
         std::fputs("No Megalo source file specified.\n", stderr);
         return false;
      }
      {
         QString q_path = QString::fromStdWString(params.megalo_source.wstring());
         QFile   file(q_path);
         if (!file.open(QIODevice::ReadOnly)) {
            auto error = file.errorString().toStdString();
            std::fprintf(stderr, "Failed to open the Megalo source file: %s\n", error.c_str());
            return false;
         }
         code = QString(file.readAll());
      }

      auto& editor  = ReachEditorState::get();
      auto* variant = editor.variant();
      assert(variant != nullptr);
      auto* mp = variant->get_multiplayer_data();
      if (!mp) {
         std::fputs("This game variant is not a multiplayer variant, and so cannot have Megalo code.\n", stderr);
         return false;
      }

      Megalo::Compiler compiler(*mp);
      compiler.parse(code);

      for (auto& item : compiler.get_fatal_errors()) {
         auto text = item.text.toStdString();
         std::fprintf(stdout, "[FATAL] Line %d Col %d: %s\n", item.location.line, item.location.col(), text.c_str());
      }
      for (auto& item : compiler.get_non_fatal_errors()) {
         auto text = item.text.toStdString();
         std::fprintf(stdout, "[ERROR] Line %d Col %d: %s\n", item.location.line, item.location.col(), text.c_str());
      }
      for (auto& item : compiler.get_warnings()) {
         auto text = item.text.toStdString();
         std::fprintf(stdout, "[WARN:] Line %d Col %d: %s\n", item.location.line, item.location.col(), text.c_str());
      }
      for (auto& item : compiler.get_notices()) {
         auto text = item.text.toStdString();
         std::fprintf(stdout, "[NOTE:] Line %d Col %d: %s\n", item.location.line, item.location.col(), text.c_str());
      }

      if (compiler.has_errors()) {
         std::fputs("Compilation failed.\n", stdout);
         return false;
      }

      {
         auto& list = compiler.get_unresolved_string_references();
         if (!list.empty()) {
            std::fprintf(stdout, "Creating %zu new strings.\n", list.size());
            for (auto& item : list) {
               item.pending.action = Megalo::Compiler::unresolved_string_pending_action::create;
            }
            compiler.handle_unresolved_string_references();
         }
      }
      compiler.apply();

      return true;
   }

   extern bool save_game_variant(const command_line_params& params) {
      auto path = params.game_variant.output;
      if (path.empty()) {
         path = params.game_variant.input;
         assert(!path.empty());
      }
      QString q_path = QString::fromStdWString(path.wstring());

      QSaveFile file(q_path);
      if (!file.open(QIODevice::WriteOnly)) {
         auto error = file.errorString().toStdString();
         std::fprintf(stderr, "Unable to open destination file for writing: %s\n", error.c_str());
         return false;
      }

      GameVariantSaveProcess save_process;
      if (q_path.endsWith(".mglo", Qt::CaseInsensitive)) {
         save_process.set_flag(GameVariantSaveProcess::flag::save_bare_mglo);
      }

      ReachEditorState::get().variant()->write(save_process);
      if (save_process.variant_is_editor_only()) {
         std::fputs("The updated game variant exceeds Reach's file format limits. Refusing to save changes.\n", stderr);
         file.cancelWriting();
         return false;
      }

      QDataStream out(&file);
      out.setVersion(QDataStream::Qt_4_5);
      out.writeRawData((const char*)save_process.writer.bytes.data(), save_process.writer.bytes.get_bytespan());
      file.commit();

      std::fputs("Updated game variant has been saved.\n", stderr);
      return true;
   }
}
