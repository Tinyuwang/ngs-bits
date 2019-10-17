#ifndef CNVWIDGET_H
#define CNVWIDGET_H

#include <QWidget>
#include <QTableWidgetItem>
#include "CnvList.h"
#include "GeneSet.h"
#include "FilterWidget.h"

namespace Ui {
class CnvWidget;
}

///Widget for visualization and filtering of CNVs.
class CnvWidget
	: public QWidget
{
	Q_OBJECT

public:
	CnvWidget(QString gsvar_file, AnalysisType analysis_type, FilterWidget* filter_widget, const GeneSet& het_hit_genes, QHash<QByteArray, BedFile>& cache, QWidget* parent = 0);
	~CnvWidget();

signals:
	void openRegionInIGV(QString region);

private slots:
	void cnvDoubleClicked(QTableWidgetItem* item);
	void applyFilters(bool debug_time=false);
	void copyToClipboard();
	void showContextMenu(QPoint p);
	void openLink(int row, int col);
	void updateQuality();
	void editQuality();
	void showQcMetricHistogram();

private:
	void loadCNVs(QString filename);
	void disableGUI();
	void addInfoLine(QString text);
	void updateStatus(int shown);
	void showSpecialTable(QString col, QString text, QByteArray url_prefix);
	QTableWidgetItem* createItem(QString text, int alignment = Qt::AlignLeft|Qt::AlignTop);

	Ui::CnvWidget* ui;
	QString ps_id; //processed sample database ID. '' if unknown of NGSD is disabled.
	CnvList cnvs;
	QStringList special_cols;
	FilterWidget* var_filters;
	GeneSet var_het_genes;
	QHash<QByteArray, BedFile>& gene2region_cache;
};

#endif // CNVWIDGET_H
