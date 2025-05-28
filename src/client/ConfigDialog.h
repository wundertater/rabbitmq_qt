#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include "ConfigManager/ConfigManager.h"

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>

class ConfigDialog : public QDialog {
  Q_OBJECT

public:
  explicit ConfigDialog(std::shared_ptr<ConfigManager>, QWidget *parent = nullptr);

  bool connectionSettingsChanged() const { return m_connectionSettingsChanged; }

private slots:
  void onSave();

private:
  QLineEdit *m_hostEdit = nullptr;
  QLineEdit *m_portEdit = nullptr;
  QLineEdit *m_loginEdit = nullptr;
  QLineEdit *m_passwordEdit = nullptr;
  QLineEdit *m_exchangeEdit = nullptr;
  QLineEdit *m_vhostEdit = nullptr;
  QLineEdit *m_requestQueueEdit = nullptr;
  QLineEdit *m_responseQueueEdit = nullptr;
  QLineEdit *m_heartbeatEdit = nullptr;

  QCheckBox *m_loggingEnabledCheckbox = nullptr;
  QLineEdit *m_logFilePathEdit = nullptr;
  QLineEdit *m_logLevelEdit = nullptr;

  std::shared_ptr<ConfigManager> m_configManager;
  bool m_connectionSettingsChanged = false;
};

#endif // CONFIGDIALOG_H
