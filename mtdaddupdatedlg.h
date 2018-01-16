#ifndef MTDADDUPDATEDLG_H
#define MTDADDUPDATEDLG_H
#include <QDialog>
#include "commondefs.h"
namespace Ui {
class MTDAddUpdateDlg;
}
class MTDAddUpdateDlg : public QDialog
{
        Q_OBJECT
public:
        explicit MTDAddUpdateDlg(QWidget *parent,
                                                         QStringList& names,
                                                         const MTDevice& mtd,
                                                         bool update);
        ~MTDAddUpdateDlg();
        const MTDevice& mtdevice() const { return m_mtd; }
private slots:
        void on_btnOK_clicked();
        void on_btnCambiar_clicked();
        void on_btnLimpiar_clicked();
private:
        Ui::MTDAddUpdateDlg *ui;
        MTDevice m_mtd;
        QStringList m_names;
        bool m_update;
};
#endif // MTDADDUPDATEDLG_H
