#include "reference_popup.h"
#include <QDesktopServices>
#include <QEvent>
#include <QFile>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QUrl>

namespace {
   static const QUrl ce_docs_home_url(QStringLiteral("qrc:/Help/script/syntax.html"));
   static const QString ce_docs_fallback_markdown = QStringLiteral(
      R"(# Megalo Docs Not Available

The full offline docs were not found in the app resources.

Expected resource:

- `:/Help/script/syntax.html`
)");
   static const QString ce_not_found_markdown = QStringLiteral(
      R"(# Page Not Found

The selected docs page could not be loaded from in-app resources.

Path:

- `%1`
)");

   static QString _label_for_url(const QUrl& url) {
      QString path = url.path();
      if (path.isEmpty())
         return QStringLiteral("Megalo Docs");
      int slash = path.lastIndexOf('/');
      if (slash >= 0)
         path = path.mid(slash + 1);
      if (path.isEmpty())
         return QStringLiteral("Megalo Docs");
      if (path.endsWith(QStringLiteral(".html"), Qt::CaseInsensitive))
         path.chop(5);
      path.replace('-', ' ');
      path.replace('_', ' ');
      if (!path.isEmpty())
         path[0] = path[0].toUpper();
      return path;
   }

   static QString _base_href_for_source(const QUrl& source) {
      if (source.scheme().toLower() != QStringLiteral("qrc"))
         return QString();
      auto qrc_path = QStringLiteral(":%1").arg(source.path());
      QFile file(qrc_path);
      if (!file.open(QIODevice::ReadOnly))
         return QString();
      auto html = QString::fromUtf8(file.readAll());
      static const QRegularExpression base_re(QStringLiteral(R"(<base[^>]*href\s*=\s*["']([^"']+)["'][^>]*>)"), QRegularExpression::CaseInsensitiveOption);
      auto match = base_re.match(html);
      if (match.hasMatch())
         return match.captured(1).trimmed();
      return QString();
   }
}

ScriptEditorReferencePopup::ScriptEditorReferencePopup(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   this->setModal(false);
   this->setWindowModality(Qt::NonModal);
   this->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
   this->setWindowFlag(Qt::Tool, true);
   this->setWindowTitle(tr("Megalo Docs"));

   this->ui.referenceText->setOpenExternalLinks(false);
   this->ui.referenceText->setOpenLinks(false);
   this->ui.referenceText->viewport()->installEventFilter(this);
   QObject::connect(this->ui.referenceText, &QTextBrowser::anchorClicked, this, [this](const QUrl& url) {
      this->openHref(url.toString());
   });
   QObject::connect(this->ui.referenceText, &QTextBrowser::sourceChanged, this, [this](const QUrl& url) {
      QString label = _label_for_url(url);
      this->ui.labelTitle->setText(label);
      this->setWindowTitle(tr("Megalo Docs - %1").arg(label));
      this->ui.buttonBack->setEnabled(this->ui.referenceText->isBackwardAvailable());
      this->ui.buttonForward->setEnabled(this->ui.referenceText->isForwardAvailable());
   });
   QObject::connect(this->ui.buttonBack, &QPushButton::clicked, this->ui.referenceText, &QTextBrowser::backward);
   QObject::connect(this->ui.buttonForward, &QPushButton::clicked, this->ui.referenceText, &QTextBrowser::forward);
   QObject::connect(this->ui.buttonHome, &QPushButton::clicked, this, [this]() {
      this->ui.referenceText->setSource(ce_docs_home_url);
   });
   QObject::connect(this->ui.buttonClose, &QPushButton::clicked, this, [this]() {
      this->hide();
   });

   this->ui.buttonBack->setEnabled(false);
   this->ui.buttonForward->setEnabled(false);
   if (QFile::exists(QStringLiteral(":/Help/script/syntax.html"))) {
      this->ui.referenceText->setSource(ce_docs_home_url);
   } else {
      this->ui.referenceText->setMarkdown(ce_docs_fallback_markdown);
   }
}

bool ScriptEditorReferencePopup::eventFilter(QObject* watched, QEvent* event) {
   if (watched == this->ui.referenceText->viewport() && event->type() == QEvent::MouseButtonRelease) {
      auto* mouse = static_cast<QMouseEvent*>(event);
      if (mouse->button() == Qt::LeftButton) {
         QString href = this->ui.referenceText->anchorAt(mouse->position().toPoint());
         if (!href.isEmpty()) {
            this->openHref(href);
            return true;
         }
      }
   }
   return QDialog::eventFilter(watched, event);
}

void ScriptEditorReferencePopup::openHref(const QString& href) {
   auto target = href.trimmed();
   if (target.isEmpty())
      return;

   auto maybe_url = QUrl(target);
   auto scheme = maybe_url.scheme().toLower();
   if (scheme == QStringLiteral("http") || scheme == QStringLiteral("https")) {
      QDesktopServices::openUrl(maybe_url);
      return;
   }

   QUrl current = this->ui.referenceText->source();
   QUrl base = current.adjusted(QUrl::RemoveFilename);
   auto base_href = _base_href_for_source(current);
   if (!base_href.isEmpty())
      base = base.resolved(QUrl(base_href));

   QUrl final = base.resolved(QUrl(target));
   if (final.scheme().isEmpty() && final.path().startsWith('/'))
      final.setScheme(QStringLiteral("qrc"));

   if (final.scheme().toLower() == QStringLiteral("qrc")) {
      auto check = final;
      check.setFragment(QString());
      auto resource_path = QStringLiteral(":%1").arg(check.path());
      if (!QFile::exists(resource_path)) {
         this->ui.referenceText->setMarkdown(ce_not_found_markdown.arg(resource_path.toHtmlEscaped()));
         return;
      }
   }
   this->ui.referenceText->setSource(final);
}
