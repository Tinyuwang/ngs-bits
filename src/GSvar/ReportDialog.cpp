#include "Exceptions.h"
#include "ReportDialog.h"
#include "NGSD.h"
#include <QTableWidgetItem>
#include <QBitArray>
#include <QPushButton>


ReportDialog::ReportDialog(QString filename, QWidget* parent)
	: QDialog(parent)
	, ui_()
	, filename_(filename)
	, labels_()
{
	//variant header labels
	labels_ << "" << "chr" << "start" << "end" << "ref" << "obs" << "ihdb_allsys_hom" << "ihdb_allsys_het" << "genotype" << "gene" << "variant_type" << "coding_and_splicing";

	//GUI
	ui_.setupUi(this);
	ui_.vars->setColumnCount(labels_.count());
	ui_.vars->setHorizontalHeaderLabels(labels_);

	ui_.outcome->addItems(NGSD().getEnum("diag_status", "outcome"));
	ui_.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
	connect(ui_.outcome, SIGNAL(currentTextChanged(QString)), this, SLOT(outcomeChanged(QString)));
}

void ReportDialog::addVariants(const VariantList& variants, const QBitArray& visible)
{
	int class_idx = variants.annotationIndexByName("classification", true, false);

	ui_.vars->setRowCount(visible.count(true));
	int row = 0;
	for (int i=0; i<variants.count(); ++i)
	{
		if (!visible[i]) continue;

		const Variant& variant = variants[i];
		ui_.vars->setItem(row, 0, new QTableWidgetItem(""));
		ui_.vars->item(row, 0)->setFlags(Qt::ItemIsUserCheckable|Qt::ItemIsEnabled|Qt::ItemIsTristate);
		ui_.vars->item(row, 0)->setCheckState(Qt::PartiallyChecked);
		if (class_idx!=-1 && variant.annotations()[class_idx]=="1")
		{
			ui_.vars->item(row, 0)->setCheckState(Qt::Unchecked);
			ui_.vars->item(row, 0)->setToolTip("Unchecked because of classification '1'.");
		}
		ui_.vars->item(row, 0)->setData(Qt::UserRole, i);
		ui_.vars->setItem(row, 1, new QTableWidgetItem(QString(variant.chr().str())));
		ui_.vars->setItem(row, 2, new QTableWidgetItem(QString::number(variant.start())));
		ui_.vars->setItem(row, 3, new QTableWidgetItem(QString::number(variant.end())));
		ui_.vars->setItem(row, 4, new QTableWidgetItem(variant.ref()));
		ui_.vars->setItem(row, 5, new QTableWidgetItem(variant.obs()));
		for (int j=6; j<labels_.count(); ++j)
		{
			int index = variants.annotationIndexByName(labels_[j], true, false);
			if (index==-1 && labels_[j]!="genotype")
			{
				THROW(ArgumentException, "Report dialog: Could not find variant annotation '" + labels_[j] + "'!");
			}
			else if (index==-1 && labels_[j]=="genotype")
			{
				ui_.vars->setItem(row, j, new QTableWidgetItem("n/a"));
			}
			else
			{
				ui_.vars->setItem(row, j, new QTableWidgetItem(variant.annotations().at(index)));
			}
		}

		++row;
	}

	ui_.vars->resizeColumnsToContents();
}

void ReportDialog::setTargetRegionSelected(bool is_selected)
{
	if (!is_selected)
	{
		ui_.details_cov->setChecked(false);
		ui_.details_cov->setEnabled(false);
	}
}

QVector< QPair<int, bool> > ReportDialog::selectedIndices() const
{
	QVector< QPair<int, bool> > output;

	for (int i=0; i<ui_.vars->rowCount(); ++i)
	{
		QTableWidgetItem* item = ui_.vars->item(i, 0);
		if (item->checkState()==Qt::Checked || item->checkState()==Qt::PartiallyChecked)
		{

			output.append(qMakePair(item->data(Qt::UserRole).toInt(), item->checkState()==Qt::Checked));
		}
	}

	return output;
}

bool ReportDialog::detailsCoverage() const
{
	return ui_.details_cov->isChecked();
}

bool ReportDialog::detailsVariants() const
{
	return ui_.details_var->isChecked();
}

QString ReportDialog::outcome() const
{
	return ui_.outcome->currentText();
}

void ReportDialog::outcomeChanged(QString text)
{
	ui_.buttonBox->button(QDialogButtonBox::Ok)->setEnabled(text!="n/a");
}

void ReportDialog::on_outcome_submit_clicked(bool)
{
	NGSD().setReportOutcome(filename_, outcome());
}
