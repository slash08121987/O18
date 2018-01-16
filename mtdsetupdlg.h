#ifndef MTDSETUPDLG_H
#define MTDSETUPDLG_H
#include <QDialog>
#include <QListWidgetItem>
#include <QIcon>
#include "commondefs.h"
#include "o18model.h"
namespace Ui {
class MTDSetupDlg;
}
class MTDSetupDlg : public QDialog
{
        Q_OBJECT
public:
        explicit MTDSetupDlg(QWidget *parent, const  mtdevices_t& mtds);
        ~MTDSetupDlg();
        const mtdevices_t& devices() const { return m_devices; }
private slots:
        void on_btnAdd_clicked();
        void on_btnUpdate_clicked();
        void on_btnDel_clicked();
        void on_wgtList_currentRowChanged(int currentRow);
        void on_wgtList_itemSelectionChanged();
private:
        static QListWidgetItem* create_mtd_item(const MTDevice& mtd);
        static QString create_mtd_text(const MTDevice& mtd);
        static QIcon create_mtd_icon(const MTDevice& mtd);
private:
        Ui::MTDSetupDlg *ui;
        mtdevices_t m_devices;
        QStringList m_names;
};
#endif // MTDSETUPDLG_H
