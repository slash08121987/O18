#include "mtdsetupdlg.h"
#include "ui_mtdsetupdlg.h"
#include <QMessageBox>
#include <QListWidgetItem>
#include <QPixmap>
#include "mtdaddupdatedlg.h"
static const int ROWHEIGHT = 24;
MTDSetupDlg::MTDSetupDlg(QWidget *parent, const mtdevices_t& mtds)
        : QDialog(parent)
        , ui(new Ui::MTDSetupDlg)
        , m_devices(mtds)
{
        ui->setupUi(this);
        setWindowFlags( windowFlags() & ~(Qt::WindowContextHelpButtonHint));
        for(size_t i = 0; i != m_devices.size(); ++i)
        {
                QListWidgetItem* item = create_mtd_item(m_devices[i]);
                ui->wgtList->addItem(item);
                m_names << m_devices[i].name;
        }
        if( !m_devices.empty() )
                ui->wgtList->setCurrentRow(0);
}
MTDSetupDlg::~MTDSetupDlg()
{
        delete ui;
}
void MTDSetupDlg::on_btnAdd_clicked()
{
        MTDevice mtd;
        if( m_devices.empty() )
        {
                mtd.height = 0.1;
                mtd.dpressure = 0.1;
                mtd.throughput = 1.;
        }
        else
                mtd = *m_devices.rbegin();
        MTDAddUpdateDlg dlg(this, m_names, mtd, false);
        if( dlg.exec() )
        {
                const MTDevice& mtd = dlg.mtdevice();
                m_devices.push_back(mtd);
                m_names << mtd.name;
                ui->wgtList->addItem(create_mtd_item(mtd));
                if( ui->wgtList->currentRow() < 0 )
                        ui->wgtList->setCurrentRow( m_devices.size() - 1 );
        }
}
void MTDSetupDlg::on_btnUpdate_clicked()
{
        int index = ui->wgtList->currentRow();
        if( index < 0 || index >= (int)m_devices.size() )
                return;
        const MTDevice& mtd = m_devices[index];
        MTDAddUpdateDlg dlg(this, m_names, mtd, true);
        if( dlg.exec() )
        {
                const MTDevice& mtd = dlg.mtdevice();
                if( m_devices[index] != mtd )
                {
                        QListWidgetItem* wi = ui->wgtList->item(index);
                        wi->setText(create_mtd_text(mtd));
                        wi->setIcon(create_mtd_icon(mtd));
                }
        }
}
void MTDSetupDlg::on_btnDel_clicked()
{
        int index = ui->wgtList->currentRow();
        if( index < 0 || index >= (int)m_devices.size() )
                return;
        const MTDevice& mtd = m_devices[index];
        if( QMessageBox::question(this, this->windowTitle(),
                                                          tr("Really delete MTD \"%1\" from list?").arg(mtd.name),
                                                          QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes )
        {
                m_devices.erase(m_devices.begin() + index);
                m_names.erase(m_names.begin() + index);
                delete ui->wgtList->item(index);
        }
}
void MTDSetupDlg::on_wgtList_currentRowChanged(int currentRow)
{
        ui->btnDel->setEnabled( currentRow >= 0 );
        ui->btnUpdate->setEnabled( currentRow >= 0 );
}
void MTDSetupDlg::on_wgtList_itemSelectionChanged()
{
        on_wgtList_currentRowChanged(ui->wgtList->currentRow());
}
QListWidgetItem *MTDSetupDlg::create_mtd_item(const MTDevice& mtd)
{
        QListWidgetItem* item = new QListWidgetItem(create_mtd_icon(mtd), create_mtd_text(mtd));
        item->setSizeHint(QSize(item->sizeHint().width(), ROWHEIGHT));
        return item;
}
QString MTDSetupDlg::create_mtd_text(const MTDevice &mtd)
{
        return tr("%1 (%2 cm, %3 kg/m2*h, %4 Pa)").arg(mtd.name)
                        .arg(mtd.height*100.).arg(mtd.throughput).arg(mtd.dpressure * 1000);
}
QIcon MTDSetupDlg::create_mtd_icon(const MTDevice &mtd)
{
        //return QIcon( QPixmap(mtd.image.exists() ? mtd.image.absoluteFilePath() : QString(":/mtd/imagen"))
        //                          .scaled(ROWHEIGHT, ROWHEIGHT) );

        return QIcon( QPixmap(mtd.image.exists() ? mtd.image.absoluteFilePath() : QString(BASE_DIR + "/mtd/imagen.jpg"))
                                  .scaled(ROWHEIGHT, ROWHEIGHT) );
}
