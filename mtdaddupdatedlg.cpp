#include "mtdaddupdatedlg.h"
#include "ui_mtdaddupdatedlg.h"
#include <QMessageBox>
#include <QPixmap>
#include <QFileInfo>
#include <QFileDialog>
MTDAddUpdateDlg::MTDAddUpdateDlg(QWidget *parent,
                                                                  QStringList& names,
                                                                  const MTDevice& mtd,
                                                                  bool update)
        : QDialog(parent)
        , ui(new Ui::MTDAddUpdateDlg)
        , m_mtd(mtd)
        , m_names(names)
        , m_update(update)
{
        ui->setupUi(this);
        setWindowFlags( windowFlags() & ~(Qt::WindowContextHelpButtonHint));
        setFixedSize(QSize(422,156));
        ui->wgtName->setEnabled(!m_update);
        ui->sbHeight->setValue(mtd.height * 100);
        ui->sbDPressure->setValue(mtd.dpressure * 1000);
        ui->sbThroughput->setValue(mtd.throughput);
        if( mtd.image.exists() )
        {
                QPixmap image(mtd.image.absoluteFilePath());
                if( !image.isNull() )
                        ui->labelIcon->setPixmap(image);
        }
        if( m_update )
        {
                this->setWindowTitle(tr("Update MTD \"%1\"").arg(mtd.name));
                ui->leName->setText(mtd.name);
        }
        else
        {
                this->setWindowTitle(tr("Add new MTD"));
                ui->leName->setText(tr("MTD%1").arg(names.size()+1));
        }
}
MTDAddUpdateDlg::~MTDAddUpdateDlg()
{
        delete ui;
}
void MTDAddUpdateDlg::on_btnOK_clicked()
{
        if( !m_update )
        {
                if( ui->leName->text().trimmed().isEmpty() )
                {
                        QMessageBox::critical(this, this->windowTitle(), tr("The MTD name can't be empty"));
                        return;
                }
                if( m_names.indexOf(ui->leName->text()) >= 0 )
                {
                        QMessageBox::critical(this, this->windowTitle(), tr("The MTD with name \"%1\" already exists")
                                                                  .arg(ui->leName->text()));
                        return;
                }
                m_mtd.name = ui->leName->text();
        }
        m_mtd.height = ui->sbHeight->value() / 100.;
        m_mtd.throughput = ui->sbThroughput->value();
        m_mtd.dpressure = ui->sbDPressure->value() / 1000.;
        this->accept();
}
void MTDAddUpdateDlg::on_btnCambiar_clicked()
{
        QString dir = QApplication::applicationDirPath();
        if(dir.at(dir.length() - 1) != '/')
                dir.append('/');
        dir += "mtd_images/";
        if( !QDir(dir).exists() )
                dir = QApplication::applicationDirPath();
        QFileDialog dl(this, tr("Change MTD's icon"));
        dl.setAcceptMode(QFileDialog::AcceptOpen);
                dl.setDirectory(dir);
        dl.setNameFilter(tr("Image files (*.bmp *.jpg *.png)"));
        if( dl.exec() )
        {
                QString name = dl.selectedFiles().at(0);
                QPixmap image(name);
                if( image.isNull() )
                {
                        QMessageBox::critical(this, windowTitle(), tr("The '%1' is not valid image file")
                                                                  .arg(QDir::toNativeSeparators(name)));
                        return;
                }
                ui->labelIcon->setPixmap(image);
                m_mtd.image = QFileInfo(name);
        }
}
void MTDAddUpdateDlg::on_btnLimpiar_clicked()
{
        ui->labelIcon->setPixmap(QPixmap(":/mtd/imagen"));
        m_mtd.image = QFileInfo("");
}
