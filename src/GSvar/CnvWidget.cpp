#include "CnvWidget.h"
#include "ui_CnvWidget.h"
#include "Helper.h"
#include "Exceptions.h"
#include "GUIHelper.h"
#include "GSvarHelper.h"
#include "VariantDetailsDockWidget.h"
#include "NGSD.h"
#include "Settings.h"
#include "Log.h"
#include "ProcessedSampleWidget.h"
#include "Histogram.h"
#include <QMessageBox>
#include <QFileInfo>
#include <QBitArray>
#include <QClipboard>
#include <QMenu>
#include <QDesktopServices>
#include <QUrl>
#include <QDir>
#include <QInputDialog>
#include <QChartView>
QT_CHARTS_USE_NAMESPACE


CnvWidget::CnvWidget(QString gsvar_file, AnalysisType analysis_type, FilterWidget* filter_widget, const GeneSet& het_hit_genes, QHash<QByteArray, BedFile>& cache, QWidget *parent)
	: QWidget(parent)
	, ui(new Ui::CnvWidget)
	, ps_id()
	, cnvs()
	, special_cols()
	, var_het_genes(het_hit_genes)
	, gene2region_cache(cache)
{
	ui->setupUi(this);
	connect(ui->cnvs, SIGNAL(itemDoubleClicked(QTableWidgetItem*)), this, SLOT(cnvDoubleClicked(QTableWidgetItem*)));
	connect(ui->cnvs, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(showContextMenu(QPoint)));
	connect(ui->copy_clipboard, SIGNAL(clicked(bool)), this, SLOT(copyToClipboard()));
	connect(ui->filter_widget, SIGNAL(filtersChanged()), this, SLOT(applyFilters()));

	//determine processed sample ID
	if (Settings::boolean("NGSD_enabled", true) && (analysis_type==GERMLINE_SINGLESAMPLE || analysis_type==GERMLINE_TRIO))
	{
		ps_id = NGSD().processedSampleId(gsvar_file, false);
	}

	//set small variant filters
	ui->filter_widget->setVariantFilterWidget(filter_widget);

	//set up NGSD menu (before loading CNV - QC actions are inserted then)
	ui->ngsd_btn->setMenu(new QMenu());
	ui->ngsd_btn->menu()->addAction(QIcon(":/Icons/Edit.png"), "Edit quality", this, SLOT(editQuality()));
	ui->ngsd_btn->menu()->addSeparator();
	ui->ngsd_btn->setEnabled(ps_id!="");

	//load CNV data file
	QFileInfo file_info(gsvar_file);
	QString base = file_info.absolutePath() + QDir::separator() + file_info.baseName();
	QString cnv_file = base + "_cnvs_clincnv.tsv";
	if (!QFile::exists(cnv_file)) //fallback to somatic
	{
		cnv_file = base + "_clincnv.tsv";
	}
	if (!QFile::exists(cnv_file)) //fallback to CnvHunter
	{
		cnv_file = base + "_cnvs.tsv";
	}
	try
	{
		loadCNVs(cnv_file);
	}
	catch(Exception e)
	{
		addInfoLine("<font color='red'>Error parsing file:\n" + e.message() + "</font>");
		disableGUI();
	}

	//quality
	updateQuality();

	//apply filters
	applyFilters();
}

CnvWidget::~CnvWidget()
{
	delete ui;
}

void CnvWidget::cnvDoubleClicked(QTableWidgetItem* item)
{
	if (item==nullptr) return;

	int col = item->column();
	int row = item->row();
	QString col_name = item->tableWidget()->horizontalHeaderItem(col)->text();
	if (special_cols.contains(col_name))
	{
		QString text = cnvs[row].annotations()[col-4].trimmed();
		if (text.isEmpty()) return;
		QString title = col_name + " of CNV " + cnvs[row].toString();

		if (col_name=="cn_pathogenic")
		{
			showSpecialTable(title, text, "");
		}
		if (col_name=="dosage_sensitive_disease_genes")
		{
			showSpecialTable(title, text, "https://www.ncbi.nlm.nih.gov/projects/dbvar/clingen/clingen_gene.cgi?sym=");
		}
		if (col_name=="clinvar_cnvs")
		{
			showSpecialTable(title, text, "https://www.ncbi.nlm.nih.gov/clinvar/variation/");
		}
		if (col_name=="hgmd_cnvs")
		{
			showSpecialTable(title, text, "https://portal.biobase-international.com/hgmd/pro/mut.php?acc=");
		}
		if (col_name=="omim")
		{
			text = text.replace('_', ' ');
			showSpecialTable(title, text, "https://www.omim.org/entry/");
		}
	}
	else
	{
		emit openRegionInIGV(cnvs[item->row()].toString());
	}
}

void CnvWidget::addInfoLine(QString text)
{
	ui->info_messages->layout()->addWidget(new QLabel(text));

	if (text.contains(":"))
	{
		QString metric = text.split(":")[0].replace("#", "").trimmed();
		ui->ngsd_btn->menu()->addAction("Show distribution: " + metric, this, SLOT(showQcMetricHistogram()));
	}
}

void CnvWidget::disableGUI()
{
	ui->cnvs->setEnabled(false);
	ui->filter_widget->setEnabled(false);
	ui->ngsd_btn->setEnabled(false);
}

void CnvWidget::loadCNVs(QString filename)
{
	//load variants from file
	cnvs.load(filename);

	//set special columns
	special_cols = QStringList() << "cn_pathogenic" << "dosage_sensitive_disease_genes" << "clinvar_cnvs" << "hgmd_cnvs" << "omim";

	//show comments
	foreach(const QByteArray& comment, cnvs.comments())
	{
		if (cnvs.type()==CnvListType::CLINCNV_GERMLINE_SINGLE || cnvs.type()==CnvListType::CLINCNV_GERMLINE_MULTI)
		{
			if (comment.contains("Analysis finished on")) continue;
			if (comment.contains("was it outlier after clustering")) continue;
			if (comment.contains("fraction of outliers")) continue;
		}
		addInfoLine(comment);
	}

	//add generic annotations
	QVector<int> annotation_indices;
	for(int i=0; i<cnvs.annotationHeaders().count(); ++i)
	{
		QByteArray header = cnvs.annotationHeaders()[i];

		//add column
		ui->cnvs->setColumnCount(ui->cnvs->columnCount() + 1);

		//create item
		QTableWidgetItem* item = new QTableWidgetItem(QString(header));
		QStringList tooltip_lines;

		QByteArray header_desc = cnvs.headerDescription(header);
		if (!header_desc.isEmpty())
		{
			tooltip_lines << header_desc;
		}
		if (special_cols.contains(header))
		{
			item->setIcon(QIcon("://Icons/Table.png"));
			tooltip_lines << "Double click table cell to open table view of annotations";
		}
		item->setToolTip(tooltip_lines.join('\n'));

		//set header
		ui->cnvs->setHorizontalHeaderItem(ui->cnvs->columnCount() -1, item);

		annotation_indices.append(i);
	}

	//show variants
	const GeneSet& imprinting_genes = GSvarHelper::impritingGenes();
	ui->cnvs->setRowCount(cnvs.count());
	for (int r=0; r<cnvs.count(); ++r)
	{
		ui->cnvs->setItem(r, 0, createItem(cnvs[r].toString()));
		ui->cnvs->setItem(r, 1, createItem(QString::number(cnvs[r].size()/1000.0, 'f', 3), Qt::AlignRight|Qt::AlignTop));
		QString regions = QString::number(cnvs[r].regions());
		if (regions=="0") regions="n/a";
		ui->cnvs->setItem(r, 2, createItem(regions, Qt::AlignRight|Qt::AlignTop));

		GeneSet genes = cnvs[r].genes();
		QTableWidgetItem* item = createItem(QString(genes.join(',')));
		if (genes.intersectsWith(imprinting_genes))
		{
			item->setBackgroundColor(Qt::yellow);
			item->setToolTip("Imprinting gene");
		}
		ui->cnvs->setItem(r, 3, item);

		int c = 4;
		foreach(int index, annotation_indices)
		{
			QTableWidgetItem* item = createItem(cnvs[r].annotations()[index]);
			//special handling for OMIM
			if (cnvs.annotationHeaders()[index]=="omim")
			{
				item->setText(item->text().replace("_", " "));
			}
			item->setToolTip(item->text().replace("],","]\n"));
			ui->cnvs->setItem(r, c++, item);
		}
	}

	//resize columns
	GUIHelper::resizeTableCells(ui->cnvs, 200);
}

void CnvWidget::applyFilters(bool debug_time)
{
	const int rows = cnvs.count();
	FilterResult filter_result(rows);

	try
	{
		//apply main filter
		QTime timer;
		timer.start();

		const FilterCascade& filter_cascade = ui->filter_widget->filters();
		//set comp-het gene list the first time the filter is applied
		for(int i=0; i<filter_cascade.count(); ++i)
		{
			const FilterCnvCompHet* comphet_filter = dynamic_cast<const FilterCnvCompHet*>(filter_cascade[i].data());
			if (comphet_filter!=nullptr && comphet_filter->hetHitGenes().count()!=var_het_genes.count())
			{
				comphet_filter->setHetHitGenes(var_het_genes);
			}
		}
		filter_result = filter_cascade.apply(cnvs, false, debug_time);
		ui->filter_widget->markFailedFilters();

		if (debug_time)
		{
			Log::perf("Applying annotation filters took ", timer);
			timer.start();
		}

		//filter by genes
		GeneSet genes = ui->filter_widget->genes();
		if (!genes.isEmpty())
		{
			QByteArray genes_joined = genes.join('|');

			if (genes_joined.contains("*")) //with wildcards
			{
				QRegExp reg(genes_joined.replace("-", "\\-").replace("*", "[A-Z0-9-]*"));
				for(int r=0; r<rows; ++r)
				{
					if (!filter_result.flags()[r]) continue;

					bool match_found = false;
					foreach(const QByteArray& cnv_gene, cnvs[r].genes())
					{
						if (reg.exactMatch(cnv_gene))
						{
							match_found = true;
							break;
						}
					}
					filter_result.flags()[r] = match_found;
				}
			}
			else //without wildcards
			{
				for(int r=0; r<rows; ++r)
				{
					if (!filter_result.flags()[r]) continue;

					filter_result.flags()[r] = cnvs[r].genes().intersectsWith(genes);
				}
			}
		}

		//filter by ROI
		QString roi_file = ui->filter_widget->targetRegion();
		if (roi_file!="")
		{
			BedFile roi;
			roi.load(roi_file);
			for(int r=0; r<rows; ++r)
			{
				if (!filter_result.flags()[r]) continue;

				filter_result.flags()[r] = roi.overlapsWith(cnvs[r].chr(), cnvs[r].start(), cnvs[r].end());
			}
		}

		//filter by region
		QString region_text = ui->filter_widget->region();
		BedLine region = BedLine::fromString(region_text);
		if (!region.isValid()) //check if valid chr
		{
			Chromosome chr(region_text);
			if (chr.isNonSpecial())
			{
				region.setChr(chr);
				region.setStart(1);
				region.setEnd(999999999);
			}
		}
		if (region.isValid()) //valid region (chr,start, end or only chr)
		{
			for(int r=0; r<rows; ++r)
			{
				if (!filter_result.flags()[r]) continue;

				filter_result.flags()[r] = region.overlapsWith(cnvs[r].chr(), cnvs[r].start(), cnvs[r].end());
			}
		}

		//filter by phenotype (via genes, not genomic regions)
		QList<Phenotype> phenotypes = ui->filter_widget->phenotypes();
		if (!phenotypes.isEmpty())
		{
			//convert phenotypes to genes
			NGSD db;
			GeneSet pheno_genes;
			foreach(const Phenotype& pheno, phenotypes)
			{
				pheno_genes << db.phenotypeToGenes(pheno, true);
			}

			//convert genes to ROI (using a cache to speed up repeating queries)
			BedFile pheno_roi;
			foreach(const QByteArray& gene, pheno_genes)
			{
				if (!gene2region_cache.contains(gene))
				{
					BedFile tmp = db.geneToRegions(gene, Transcript::ENSEMBL, "gene", true);
					tmp.clearAnnotations();
					tmp.extend(5000);
					tmp.merge();
					gene2region_cache[gene] = tmp;
				}
				pheno_roi.add(gene2region_cache[gene]);
			}
			pheno_roi.merge();

			for(int r=0; r<rows; ++r)
			{
				if (!filter_result.flags()[r]) continue;

				filter_result.flags()[r] = pheno_roi.overlapsWith(cnvs[r].chr(), cnvs[r].start(), cnvs[r].end());
			}
		}

		//filter annotations by text
		QByteArray text = ui->filter_widget->text().trimmed().toLower();
		if (text!="")
		{
			for(int r=0; r<rows; ++r)
			{
				if (!filter_result.flags()[r]) continue;

				bool match = false;
				foreach(const QByteArray& anno, cnvs[r].annotations())
				{
					if (anno.toLower().contains(text))
					{
						match = true;
						break;
					}
				}
				filter_result.flags()[r] = match;
			}
		}

	}
	catch(Exception& e)
	{
		QMessageBox::warning(this, "Filtering error", e.message() + "\nA possible reason for this error is an outdated variant list.\nTry re-annotating the NGSD columns.\n If re-annotation does not help, please re-analyze the sample (starting from annotation) in the sample information dialog!");

		filter_result = FilterResult(cnvs.count(), false);
	}

	//update GUI
	for(int r=0; r<rows; ++r)
	{
		ui->cnvs->setRowHidden(r, !filter_result.flags()[r]);
	}
	updateStatus(filter_result.countPassing());
}

void CnvWidget::copyToClipboard()
{
	GUIHelper::copyToClipboard(ui->cnvs);
}

void CnvWidget::showContextMenu(QPoint p)
{
	//make sure a row was clicked
	int row = ui->cnvs->indexAt(p).row();
	if (row==-1) return;

	//create menu
	QMenu menu;
	menu.addAction(QIcon("://Icons/Decipher.png"), "Open in Decipher browser");
	menu.addAction(QIcon("://Icons/DGV.png"), "Open in DGV");
	menu.addAction(QIcon("://Icons/UCSC.png"), "Open in UCSC browser");
	menu.addAction(QIcon("://Icons/UCSC.png"), "Open in UCSC browser (override tracks)");

	//exec menu
	QAction* action = menu.exec(ui->cnvs->viewport()->mapToGlobal(p));
	if (action==nullptr) return;
	QString text = action->text();


	if (text=="Open in DGV")
	{
		QDesktopServices::openUrl(QUrl("http://dgv.tcag.ca/gb2/gbrowse/dgv2_hg19/?name=" + cnvs[row].toString()));
	}
	else if (text=="Open in UCSC browser")
	{
		QDesktopServices::openUrl(QUrl("https://genome.ucsc.edu/cgi-bin/hgTracks?db=hg19&position=" + cnvs[row].toString()));
	}
	else if (text=="Open in UCSC browser (override tracks)")
	{
		QDesktopServices::openUrl(QUrl("https://genome.ucsc.edu/cgi-bin/hgTracks?db=hg19&ignoreCookie=1&hideTracks=1&cytoBand=pack&refSeqComposite=dense&ensGene=dense&omimGene2=pack&geneReviews=pack&dgvPlus=squish&genomicSuperDups=squish&position=" + cnvs[row].toString()));
	}
	else if (text=="Open in Decipher browser")
	{
		QDesktopServices::openUrl(QUrl("https://decipher.sanger.ac.uk/browser#q/" + cnvs[row].toString()));
	}
}

void CnvWidget::openLink(int row, int col)
{
	auto table = qobject_cast<QTableWidget*>(sender());
	if (table==nullptr) return;
	auto item = table->item(row, col);
	if (item==nullptr) return;

	QString url = item->data(Qt::UserRole).toString();
	if (url.isEmpty()) return;

	QDesktopServices::openUrl(QUrl(url));
}

void CnvWidget::updateQuality()
{
	QString quality = "n/a";
	if  (ps_id!="")
	{
		quality = NGSD().getValue("SELECT quality FROM cnv_callset WHERE processed_sample_id=" + ps_id).toString();
	}
	ui->quality->setEnabled(ps_id!="");
	ProcessedSampleWidget::styleQualityLabel(ui->quality, quality);
}

void CnvWidget::editQuality()
{
	NGSD db;
	QStringList qualities = db.getEnum("cnv_callset", "quality");

	bool ok;
	QString quality = QInputDialog::getItem(this, "Select CNV callset quality", "quality:", qualities, qualities.indexOf(ui->quality->toolTip()), false, &ok);
	if (!ok) return;

	db.getQuery().exec("UPDATE cnv_callset SET quality='" + quality + "' WHERE processed_sample_id=" + ps_id);

	updateQuality();
}

void CnvWidget::showQcMetricHistogram()
{
	//determine metrics name
	QAction* action = qobject_cast<QAction*>(sender());
	if (action==nullptr || !action->text().contains(":")) return;
	QString metric_name = action->text().split(":")[1].trimmed();

	//get metrics
	NGSD db;
	QString sys_id = db.getValue("SELECT processing_system_id FROM processed_sample WHERE id=" + ps_id).toString();
	QVector<double> metrics = db.cnvCallsetMetrics(sys_id, metric_name);
	if (metrics.isEmpty())
	{
		QMessageBox::information(this, "CNV callset QC metrics", "No (numeric) QC entries found for metric '" + metric_name + "'!");
		return;
	}

	//determine median
	std::sort(metrics.begin(), metrics.end());
	double median = BasicStatistics::median(metrics, false);

	//create histogram
	double upper_bound = median * 2.5;
	Histogram hist(0.0, upper_bound, upper_bound/30);
	hist.inc(metrics, true);

	//show histogram
	QChartView* view = GUIHelper::histogramChart(hist, "#CNVs");
	auto dlg = GUIHelper::createDialog(view, "High-quality CNV distribution");
	dlg->exec();
}

void CnvWidget::showSpecialTable(QString col, QString text, QByteArray url_prefix)
{
	//determine headers
	auto entries = VariantDetailsDockWidget::parseDB(text, ',');
	QStringList headers;
	headers << "ID";
	foreach(const VariantDetailsDockWidget::DBEntry& entry, entries)
	{
		QList<KeyValuePair> parts = entry.splitByName();
		foreach(const KeyValuePair& pair, parts)
		{
			if (!headers.contains(pair.key))
			{
				headers << pair.key;
			}
		}
	}

	//set up table widget
	QTableWidget* table = new QTableWidget();
	table->setRowCount(entries.count());
	table->setColumnCount(headers.count());
	for(int col=0; col<headers.count(); ++col)
	{
		auto item = new QTableWidgetItem(headers[col]);
		if (col==0 && !url_prefix.isEmpty())
		{
			item->setIcon(QIcon(":/Icons/Link.png"));
			item->setToolTip("Double click cell to open external link for the entry");
		}
		table->setHorizontalHeaderItem(col, item);
	}
	if (!url_prefix.isEmpty())
	{
		connect(table, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(openLink(int,int)));
	}
	table->setEditTriggers(QAbstractItemView::NoEditTriggers);
	table->setAlternatingRowColors(true);
	table->setWordWrap(false);
	table->setSelectionMode(QAbstractItemView::SingleSelection);
	table->setSelectionBehavior(QAbstractItemView::SelectItems);
	table->verticalHeader()->setVisible(false);
	int row = 0;
	foreach(VariantDetailsDockWidget::DBEntry entry, entries)
	{
		QString id = entry.id.trimmed();
		auto item = new QTableWidgetItem(id);
		if (!url_prefix.isEmpty())
		{
			item->setData(Qt::UserRole, url_prefix + id);
		}
		table->setItem(row, 0, item);

		QList<KeyValuePair> parts = entry.splitByName();
		foreach(const KeyValuePair& pair, parts)
		{
			int col = headers.indexOf(pair.key);
			table->setItem(row, col, new QTableWidgetItem(pair.value));
		}

		++row;
	}

	//show table
	auto dlg = GUIHelper::createDialog(table, col);
	dlg->setMinimumSize(1200, 800);
	GUIHelper::resizeTableCells(table);
	dlg->exec();

	//delete
	delete table;
}
void CnvWidget::updateStatus(int shown)
{
	QString text = QString::number(shown) + "/" + QString::number(cnvs.count()) + " passing filter(s)";
	ui->status->setText(text);
}

QTableWidgetItem* CnvWidget::createItem(QString text, int alignment)
{
	QTableWidgetItem* item = new QTableWidgetItem(text);
	item->setTextAlignment(alignment);

	return item;
}


