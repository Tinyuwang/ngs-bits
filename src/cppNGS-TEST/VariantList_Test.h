#include "TestFramework.h"
#include "TestFrameworkNGS.h"
#include "VariantList.h"
#include "Settings.h"

TEST_CLASS(VariantList_Test)
{
Q_OBJECT
private slots:

	void type()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));

		I_EQUAL(vl.type(false), GERMLINE_SINGLESAMPLE);
	}

	void leftAlign()
	{
		QString ref_file = Settings::string("reference_genome", true);
		if (ref_file=="") SKIP("Test needs the reference genome!");

		VariantList vl;
		vl.load(TESTDATA("data_in/LeftAlign_in.GSvar"));
		vl.checkValid();
		vl.leftAlign(ref_file, false);
		vl.store("out/LeftAlign_out.GSvar");
		COMPARE_FILES("out/LeftAlign_out.GSvar", TESTDATA("data_out/LeftAlign_out.GSvar"));
	}

	void leftAlign_nothing_to_do()
	{
		QString ref_file = Settings::string("reference_genome", true);
		if (ref_file=="") SKIP("Test needs the reference genome!");

		VariantList vl;
		vl.load(TESTDATA("data_in/LeftAlign_in2.GSvar"));
		vl.checkValid();
		vl.leftAlign(ref_file, false);
		vl.store("out/LeftAlign_out2.GSvar");
		COMPARE_FILES("out/LeftAlign_out2.GSvar", TESTDATA("data_out/LeftAlign_out2.GSvar"));
	}

	void removeDuplicates_VCF()
	{
		VariantList vl,vl2;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));
		vl.checkValid();
		vl.sort();
		vl2.load(TESTDATA("data_in/variantList_removeDuplicates.vcf"));
		vl2.checkValid();
		vl2.removeDuplicates(true);
		//after removal of duplicates (and numerical sorting of vl), vl and vl2 should be the same
		I_EQUAL(vl.count(),vl2.count());
		for (int i=0; i<vl.count(); ++i)
		{
			S_EQUAL(vl[i].start(),vl2[i].start());
			S_EQUAL(vl[i].obs() ,vl2[i].obs());
		}
	}

	void removeDuplicates_TSV()
	{
		VariantList vl,vl2;
		vl.load(TESTDATA("data_in/variantList_removeDuplicates_in.tsv"));
		vl.checkValid();
        vl.removeDuplicates(false);
		vl2.load(TESTDATA("data_out/variantList_removeDuplicates_out.tsv"));
		vl2.checkValid();
		vl2.sort();
		//after removal of duplicates vl and vl2 should be the same
		I_EQUAL(vl.count(),vl2.count());
		for (int i=0; i<vl2.count(); ++i)
		{
			S_EQUAL(vl[i].start(),vl2[i].start());
			S_EQUAL(vl[i].obs() ,vl2[i].obs());
		}
	}

	//check that it works with empty variant lists
	void removeDuplicates_Empty()
	{
		VariantList vl;
		vl.removeDuplicates(true);
	}

	void loadFromVCF()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));
		vl.checkValid();
		I_EQUAL(vl.count(), 14);
		I_EQUAL(vl.comments().count(), 3);
		S_EQUAL(vl.sampleNames()[0], QString("./Sample_GS120297A3/GS120297A3.bam"));
		I_EQUAL(vl.annotations().count(), 27);

		VariantAnnotationDescription vad = vl.annotationDescriptionByName("ID");
		S_EQUAL(vad.name(), QString("ID"));
		X_EQUAL(vad.type(), VariantAnnotationDescription::STRING);
		S_EQUAL(vad.number(), QString("1"));
		S_EQUAL(vad.description(), QString("ID of the variant, often dbSNP rsnumber"));
		IS_FALSE(vad.sampleSpecific());

		vad = vl.annotationDescriptionByName("INDEL");
		S_EQUAL(vad.name(), QString("INDEL"));
		X_EQUAL(vad.type(), VariantAnnotationDescription::FLAG);
		S_EQUAL(vad.number(), QString("0"));
		S_EQUAL(vad.description(), QString("Indicates that the variant is an INDEL."));
		IS_FALSE(vad.sampleSpecific());

		vad = vl.annotationDescriptionByName("DP4");
		S_EQUAL(vad.name(), QString("DP4"));
		I_EQUAL(vad.type(), VariantAnnotationDescription::INTEGER);
		S_EQUAL(vad.number(), QString("4"));
		S_EQUAL(vad.description(), QString("# high-quality ref-forward bases, ref-reverse, alt-forward and alt-reverse bases"));
		IS_FALSE(vad.sampleSpecific());

		vad = vl.annotationDescriptionByName("PL",true);
		S_EQUAL(vad.name(), QString("PL"));
		S_EQUAL(vad.number(), QString("G"));
		S_EQUAL(vad.description(), QString("List of Phred-scaled genotype likelihoods"));
		IS_TRUE(vad.sampleSpecific());
		I_EQUAL(vad.type(), VariantAnnotationDescription::INTEGER);

		I_EQUAL(vl.filters().count(), 2);
		S_EQUAL(vl.filters()["q10"], QString("Quality below 10"));
		S_EQUAL(vl.filters()["s50"], QString("Less than 50% of samples have data"));

		X_EQUAL(vl[0].chr(), Chromosome("chr17"));
		I_EQUAL(vl[0].start(), 72196817);
		I_EQUAL(vl[0].end(), 72196817);
		S_EQUAL(vl[0].ref(), Sequence("G"));
		S_EQUAL(vl[0].obs(), Sequence("GA"));
		S_EQUAL(vl[0].annotations().at(3), QByteArray("TRUE"));
		S_EQUAL(vl[0].annotations().at(8), QByteArray("4,3,11,11"));
		S_EQUAL(vl[0].annotations().at(26), QByteArray("255,0,123"));
		I_EQUAL(vl[0].filters().count(), 0);

		I_EQUAL(vl[11].filters().count(), 1);
		S_EQUAL(vl[11].filters().at(0), QByteArray("low_DP"));

		X_EQUAL(vl[12].chr(), Chromosome("chr9"));
		I_EQUAL(vl[12].start(), 130931421);
		I_EQUAL(vl[12].end(), 130931421);
		S_EQUAL(vl[12].ref(), Sequence("G"));
		S_EQUAL(vl[12].obs(), Sequence("A"));
		S_EQUAL(vl[12].annotations().at(3), QByteArray(""));
		S_EQUAL(vl[12].annotations().at(8), QByteArray("457,473,752,757"));
		S_EQUAL(vl[12].annotations().at(26), QByteArray("255,0,255"));
		I_EQUAL(vl[12].filters().count(), 0);

		//load a second time to check initialization
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));
		vl.checkValid();
		I_EQUAL(vl.count(), 14);
		I_EQUAL(vl.annotations().count(), 27);
		I_EQUAL(vl.comments().count(), 3);
		S_EQUAL(vl.sampleNames()[0], QString("./Sample_GS120297A3/GS120297A3.bam"));
	}

	void loadFromVCF_withROI()
	{
		BedFile roi;
		roi.append(BedLine("chr17", 72196820, 72196892));
		roi.append(BedLine("chr18", 67904549, 67904670));

		VariantList vl;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"), VCF, &roi);
		vl.checkValid();
		I_EQUAL(vl.count(), 4);
		I_EQUAL(vl.comments().count(), 3);
		S_EQUAL(vl.sampleNames()[0], QString("./Sample_GS120297A3/GS120297A3.bam"));
		I_EQUAL(vl.annotations().count(), 27);

		X_EQUAL(vl[0].chr(), Chromosome("chr17"));
		I_EQUAL(vl[0].start(), 72196887);
		X_EQUAL(vl[1].chr(), Chromosome("chr17"));
		I_EQUAL(vl[1].start(), 72196892);
		X_EQUAL(vl[2].chr(), Chromosome("chr18"));
		I_EQUAL(vl[2].start(), 67904549);
		X_EQUAL(vl[3].chr(), Chromosome("chr18"));
		I_EQUAL(vl[3].start(), 67904586);
	}

	void loadFromVCF_noSampleOrFormatColumn()
	{
		VariantList vl;

		vl.load(TESTDATA("data_in/VariantList_loadFromVCF_noSample.vcf"));
		vl.checkValid();
		I_EQUAL(vl.count(), 14);
		I_EQUAL(vl.annotations().count(), 27);
		I_EQUAL(vl.comments().count(), 2);
		S_EQUAL(vl.sampleNames()[0], QString("Sample"));

		vl.clear();
		vl.load(TESTDATA("data_in/VariantList_loadFromVCF_noFormatSample.vcf"));
		vl.checkValid();
		I_EQUAL(vl.count(), 14);
		I_EQUAL(vl.annotations().count(), 27);
		I_EQUAL(vl.comments().count(), 2);
		S_EQUAL(vl.sampleNames()[0], QString("Sample"));
	}

	void loadFromVCF_undeclaredAnnotations()
	{
		VariantList vl;

		//check annotation list
		vl.load(TESTDATA("data_in/VariantList_loadFromVCF_undeclaredAnnotations.vcf"));
		vl.checkValid();
		I_EQUAL(vl.count(), 2);
		I_EQUAL(vl.annotations().count(), 18);
		QStringList names;
		foreach(VariantAnnotationHeader d, vl.annotations())
		{
			names << d.name();
		}
		S_EQUAL(names.join(","), QString("ID,QUAL,FILTER,DP,AF,RO,AO,GT,GQ,GL,DP,RO,QR,AO,QA,TRIO,CIGAR,TRIO2"));

		//check variants
		S_EQUAL(vl[0].annotations()[16], QByteArray("1X"));
		S_EQUAL(vl[1].annotations()[16], QByteArray(""));
		S_EQUAL(vl[0].annotations()[17], QByteArray(""));
		S_EQUAL(vl[1].annotations()[17], QByteArray("HET,9,0.56,WT,17,0.00,HOM,19,1.00"));
	}

	void loadFromVCF_emptyFormatAndInfo()
	{
		QString in = TESTDATA("data_in/VariantList_loadFromVCF_emptyInfoAndFormat.vcf");
		QString out = "out/VariantList_loadFromVCF_emptyInfoAndFormat.vcf";

		VariantList vl;
		vl.load(in);
		vl.checkValid();
		vl.store(out);

		COMPARE_FILES(in,out);
	}

	void storeToVCF()
	{
		//store loaded file
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));
		vl.checkValid();
		vl.store("out/VariantList_store_01.vcf");
		VCF_IS_VALID("out/VariantList_store_01.vcf")
		vl.clear();

		//reload and check that everything stayed the same
		vl.load("out/VariantList_store_01.vcf");
		vl.checkValid();
		I_EQUAL(vl.count(), 14);
		I_EQUAL(vl.comments().count(), 3);
		S_EQUAL(vl.sampleNames()[0], QString("./Sample_GS120297A3/GS120297A3.bam"));

		I_EQUAL(vl.annotations().count(), 27);
		S_EQUAL(vl.annotations()[0].name(), QString("ID"));

		VariantAnnotationDescription vad = vl.annotationDescriptionByName("ID");
		I_EQUAL(vad.type(), VariantAnnotationDescription::STRING);
		S_EQUAL(vad.number(), QString("1"));
		S_EQUAL(vad.description(), QString("ID of the variant, often dbSNP rsnumber"));
		IS_FALSE(vad.sampleSpecific());

		vad = vl.annotationDescriptionByName("INDEL");
		S_EQUAL(vad.name(), QString("INDEL"));
		I_EQUAL(vad.type(), VariantAnnotationDescription::FLAG);
		S_EQUAL(vad.number(), QString("0"));
		S_EQUAL(vad.description(), QString("Indicates that the variant is an INDEL."));
		IS_FALSE(vad.sampleSpecific());

		vad = vl.annotationDescriptionByName("DP4");
		S_EQUAL(vad.name(), QString("DP4"));
		I_EQUAL(vad.type(), VariantAnnotationDescription::INTEGER);
		S_EQUAL(vad.number(), QString("4"));
		S_EQUAL(vad.description(), QString("# high-quality ref-forward bases, ref-reverse, alt-forward and alt-reverse bases"));
		IS_FALSE(vad.sampleSpecific());

		vad = vl.annotationDescriptionByName("PL",true);
		S_EQUAL(vad.name(), QString("PL"));
		S_EQUAL(vad.number(), QString("G"));
		S_EQUAL(vad.description(), QString("List of Phred-scaled genotype likelihoods"));
		IS_TRUE(vad.sampleSpecific());
		I_EQUAL(vad.type(), VariantAnnotationDescription::INTEGER);

		I_EQUAL(vl.filters().count(), 2);
		S_EQUAL(vl.filters()["q10"], QString("Quality below 10"));
		S_EQUAL(vl.filters()["s50"], QString("Less than 50% of samples have data"));

		X_EQUAL(vl[0].chr(), Chromosome("chr17"));
		I_EQUAL(vl[0].start(), 72196817);
		I_EQUAL(vl[0].end(), 72196817);
		S_EQUAL(vl[0].ref(), Sequence("G"));
		S_EQUAL(vl[0].obs(), Sequence("GA"));
		S_EQUAL(vl[0].annotations().at(3), QByteArray("TRUE"));
		S_EQUAL(vl[0].annotations().at(8), QByteArray("4,3,11,11"));
		S_EQUAL(vl[0].annotations().at(26), QByteArray("255,0,123"));

		X_EQUAL(vl[12].chr(), Chromosome("chr9"));
		I_EQUAL(vl[12].start(), 130931421);
		I_EQUAL(vl[12].end(), 130931421);
		S_EQUAL(vl[12].ref(), Sequence("G"));
		S_EQUAL(vl[12].obs(), Sequence("A"));
		S_EQUAL(vl[12].annotations().at(3), QByteArray(""));
		S_EQUAL(vl[12].annotations().at(8), QByteArray("457,473,752,757"));
		S_EQUAL(vl[12].annotations().at(26), QByteArray("255,0,255"));
	}

	void loadFromTSV()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		vl.checkValid();
		I_EQUAL(vl.count(), 329);
		I_EQUAL(vl.annotations().count(), 30);
		S_EQUAL(vl.annotations()[0].name(), QString("NA12878_03"));
		S_EQUAL(vl.annotations()[27].name(), QString("validation"));
		I_EQUAL(vl.filters().count(), 2);
		S_EQUAL(vl.filters()["gene_blacklist"], QString("The gene(s) are contained on the blacklist of unreliable genes."));
		S_EQUAL(vl.filters()["off-target"], QString("Variant marked as 'off-target'."));

		X_EQUAL(vl[0].chr(), Chromosome("chr1"));
		I_EQUAL(vl[0].start(), 27682481);
		I_EQUAL(vl[0].end(), 27682481);
		S_EQUAL(vl[0].ref(), Sequence("G"));
		S_EQUAL(vl[0].obs(), Sequence("A"));
		S_EQUAL(vl[0].annotations().at(0), QByteArray("het"));
		S_EQUAL(vl[0].annotations().at(7), QByteArray("rs12569127"));
		S_EQUAL(vl[0].annotations().at(9), QByteArray("0.2659"));
		I_EQUAL(vl[0].filters().count(), 1);

		X_EQUAL(vl[328].chr(), Chromosome("chr20"));
		I_EQUAL(vl[328].start(), 48301146);
		I_EQUAL(vl[328].end(), 48301146);
		S_EQUAL(vl[328].ref(), Sequence("G"));
		S_EQUAL(vl[328].obs(), Sequence("A"));
		S_EQUAL(vl[328].annotations().at(0), QByteArray("hom"));
		S_EQUAL(vl[328].annotations().at(7), QByteArray("rs6512586"));
		S_EQUAL(vl[328].annotations().at(9), QByteArray("0.5178"));
		I_EQUAL(vl[328].filters().count(), 0);

		//load a second time to check initialization
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		vl.checkValid();
		I_EQUAL(vl.count(), 329);
		I_EQUAL(vl.annotations().count(), 30);
	}

	void test_backward_compatabilty_genotype_column()
	{
		//new format
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		vl.checkValid();
		SampleInfo info = vl.getSampleHeader().infoByStatus(true);
		S_EQUAL(info.id, "NA12878_03");
		S_EQUAL(info.column_name, "NA12878_03");
		I_EQUAL(info.column_index, 0);


		//old format
		vl.load(TESTDATA("data_in/old_format.GSvar"));
		vl.checkValid();
		info = vl.getSampleHeader().infoByStatus(true);
		S_EQUAL(info.id, "NA12878_03");
		S_EQUAL(info.column_name, "genotype");
		I_EQUAL(info.column_index, 0);
	}
	void loadFromTSV_withROI()
	{
		BedFile roi;
		roi.append(BedLine("chr16", 89805260, 89805978));
		roi.append(BedLine("chr19", 17379550, 17382510));

		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.GSvar"), TSV, &roi);
		I_EQUAL(vl.count(), 4);
		I_EQUAL(vl.annotations().count(), 30);
		S_EQUAL(vl.annotations()[0].name(), QString("NA12878_03"));
		S_EQUAL(vl.annotations()[27].name(), QString("validation"));
		I_EQUAL(vl.filters().count(), 2);
		S_EQUAL(vl.filters()["gene_blacklist"], QString("The gene(s) are contained on the blacklist of unreliable genes."));
		S_EQUAL(vl.filters()["off-target"], QString("Variant marked as 'off-target'."));

		X_EQUAL(vl[0].chr(), Chromosome("chr16"));
		I_EQUAL(vl[0].start(), 89805261);
		X_EQUAL(vl[1].chr(), Chromosome("chr16"));
		I_EQUAL(vl[1].start(), 89805977);
		X_EQUAL(vl[2].chr(), Chromosome("chr19"));
		I_EQUAL(vl[2].start(), 17379558);
		X_EQUAL(vl[3].chr(), Chromosome("chr19"));
		I_EQUAL(vl[3].start(), 17382505);
	}

	void storeToTSV()
	{
		//store loaded tsv file
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		vl.checkValid();
		vl.store("out/VariantList_store_01.tsv");
		vl.clear();

		//reload and check that everything stayed the same
		vl.load("out/VariantList_store_01.tsv");
		vl.checkValid();
		I_EQUAL(vl.count(), 329);

		I_EQUAL(vl.annotations().count(), 30);
		S_EQUAL(vl.annotations()[0].name(), QString("NA12878_03"));
		S_EQUAL(vl.annotations()[27].name(), QString("validation"));

		I_EQUAL(vl.filters().count(), 2);
		S_EQUAL(vl.filters()["gene_blacklist"], QString("The gene(s) are contained on the blacklist of unreliable genes."));
		S_EQUAL(vl.filters()["off-target"], QString("Variant marked as 'off-target'."));

		X_EQUAL(vl[0].chr(), Chromosome("chr1"));
		I_EQUAL(vl[0].start(), 27682481);
		I_EQUAL(vl[0].end(), 27682481);
		S_EQUAL(vl[0].ref(), Sequence("G"));
		S_EQUAL(vl[0].obs(), Sequence("A"));
		S_EQUAL(vl[0].annotations().at(0), QByteArray("het"));
		S_EQUAL(vl[0].annotations().at(7), QByteArray("rs12569127"));
		S_EQUAL(vl[0].annotations().at(9), QByteArray("0.2659"));

		X_EQUAL(vl[328].chr(), Chromosome("chr20"));
		I_EQUAL(vl[328].start(), 48301146);
		I_EQUAL(vl[328].end(), 48301146);
		S_EQUAL(vl[328].ref(), Sequence("G"));
		S_EQUAL(vl[328].obs(), Sequence("A"));
		S_EQUAL(vl[328].annotations().at(0), QByteArray("hom"));
		S_EQUAL(vl[328].annotations().at(7), QByteArray("rs6512586"));
		S_EQUAL(vl[328].annotations().at(9), QByteArray("0.5178"));

		//load a second time to check initialization
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		vl.checkValid();
		I_EQUAL(vl.count(), 329);
		I_EQUAL(vl.annotations().count(), 30);
	}

	void convertVCFtoTSV()
	{
		//store loaded vcf file
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));
		vl.checkValid();
		vl.store("out/VariantList_convertVCFtoTSV.tsv");
		vl.clear();

		//reload and check that no information became incorrect (vcf-specific things like annotation dimensions and types are still lost)
		vl.load("out/VariantList_convertVCFtoTSV.tsv");
		vl.checkValid();
		I_EQUAL(vl.count(), 14);
		I_EQUAL(vl.annotations().count(), 27);
		I_EQUAL(vl.comments().count(), 2);
		S_EQUAL(vl.annotations()[0].name(), QString("ID"));
		S_EQUAL(vl.annotationDescriptionByName("ID").description(), QString("ID of the variant, often dbSNP rsnumber"));
		S_EQUAL(vl.annotationDescriptionByName("INDEL").name(), QString("INDEL"));
		S_EQUAL(vl.annotationDescriptionByName("INDEL").description(), QString("Indicates that the variant is an INDEL."));
		S_EQUAL(vl.annotationDescriptionByName("DP4").name(), QString("DP4"));
		S_EQUAL(vl.annotationDescriptionByName("DP4").description(), QString("# high-quality ref-forward bases, ref-reverse, alt-forward and alt-reverse bases"));
		S_EQUAL(vl.annotationDescriptionByName("PL_ss").name(), QString("PL_ss"));
		S_EQUAL(vl.annotationDescriptionByName("PL_ss").description(), QString("List of Phred-scaled genotype likelihoods"));


		X_EQUAL(vl[0].chr(), Chromosome("chr17"));
		I_EQUAL(vl[0].start(), 72196817);
		I_EQUAL(vl[0].end(), 72196817);
		S_EQUAL(vl[0].ref(), Sequence("G"));
		S_EQUAL(vl[0].obs(), Sequence("GA"));
		S_EQUAL(vl[0].annotations().at(3), QByteArray("TRUE"));
		S_EQUAL(vl[0].annotations().at(8), QByteArray("4,3,11,11"));
		S_EQUAL(vl[0].annotations().at(26), QByteArray("255,0,123"));

		X_EQUAL(vl[12].chr(), Chromosome("chr9"));
		I_EQUAL(vl[12].start(), 130931421);
		I_EQUAL(vl[12].end(), 130931421);
		S_EQUAL(vl[12].ref(), Sequence("G"));
		S_EQUAL(vl[12].obs(), Sequence("A"));
		S_EQUAL(vl[12].annotations().at(3), QByteArray(""));
		S_EQUAL(vl[12].annotations().at(8), QByteArray("457,473,752,757"));
		S_EQUAL(vl[12].annotations().at(26), QByteArray("255,0,255"));
	}

	void checkThatEmptyVariantAnnotationsAreFilled()
	{
		//store loaded vcf file
		VariantList vl;
		vl.load(TESTDATA("data_in/VariantList_emptyDescriptions.vcf"));
		vl.checkValid();
		vl.store("out/VariantList_emptyDescriptions_fixed.vcf");
		VCF_IS_VALID("out/VariantList_emptyDescriptions_fixed.vcf")
		vl.clear();

		VariantList vl2;
		vl2.load("out/VariantList_emptyDescriptions_fixed.vcf");
		vl2.checkValid();
		I_EQUAL(vl2.count(), 14);
		I_EQUAL(vl2.annotations().count(), 27);
		foreach(VariantAnnotationHeader ah, vl2.annotations())
		{
			VariantAnnotationDescription ad = vl2.annotationDescriptionByName(ah.name(),!ah.sampleID().isNull());
			if (ah.name()=="GQ" || ah.name()=="MQ")
			{
				S_EQUAL(ad.description(), "no description available");
			}
			else
			{
				IS_FALSE(ad.description()=="no description available");
			}
		}
	}

	void loadFromVCF_GZ()
	{
		VariantList vl;
		VariantListFormat format = vl.load(TESTDATA("data_in/VariantList_load_zipped.vcf.gz"));
		vl.checkValid();
		I_EQUAL(format, VCF_GZ);
		I_EQUAL(vl.count(), 157);
		I_EQUAL(vl.annotations().count(), 75);
		S_EQUAL(vl.annotations()[0].name(), "ID");
		S_EQUAL(vl.annotations()[1].name(), "QUAL");
		S_EQUAL(vl.annotations()[2].name(), "FILTER");
		S_EQUAL(vl.annotations()[3].name(), "NS");
		S_EQUAL(vl.annotations()[74].name(), "EXAC_AF");

		X_EQUAL(vl[0].chr().str(), "chr1");
		I_EQUAL(vl[0].start(), 27687466);
		I_EQUAL(vl[0].end(), 27687466);
		S_EQUAL(vl[0].ref(), Sequence("G"));
		S_EQUAL(vl[0].obs(), Sequence("T"));
		S_EQUAL(vl[0].annotations().at(0), "rs35659744");
		S_EQUAL(vl[0].annotations().at(1), "11836.9");
		S_EQUAL(vl[0].annotations().at(2), ".");
		S_EQUAL(vl[0].annotations().at(3), "1");
		S_EQUAL(vl[0].annotations().at(74), "0.223");

		X_EQUAL(vl[156].chr().str(), "chr20");
		I_EQUAL(vl[156].start(), 48301146);
		I_EQUAL(vl[156].end(), 48301146);
		S_EQUAL(vl[156].ref(), Sequence("G"));
		S_EQUAL(vl[156].obs(), Sequence("A"));
		S_EQUAL(vl[156].annotations().at(0), "rs6512586");
		S_EQUAL(vl[156].annotations().at(1), "39504.2");
		S_EQUAL(vl[156].annotations().at(2), ".");
		S_EQUAL(vl[156].annotations().at(3), "1");
		S_EQUAL(vl[156].annotations().at(74), "0.516");
	}

	void annotationIndexByName()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		vl.checkValid();
		I_EQUAL(vl.annotationIndexByName("NA12878_03", true, false), 0);
		I_EQUAL(vl.annotationIndexByName("NA12878_03", false, false), 0);
		I_EQUAL(vl.annotationIndexByName("validation", true, false), 27);
		I_EQUAL(vl.annotationIndexByName("validation", false, false), 27);
		I_EQUAL(vl.annotationIndexByName("ESP_", false, false), 12);
		I_EQUAL(vl.annotationIndexByName("fathmm-", false, false), 16);
	}

	void vepIndexByName()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.vcf"));
		I_EQUAL(vl.vepIndexByName("Allele", false), 0);
		I_EQUAL(vl.vepIndexByName("Consequence", false), 1);
		I_EQUAL(vl.vepIndexByName("IMPACT", false), 2);
		I_EQUAL(vl.vepIndexByName("HGMD_PHEN", false), 59);
		I_EQUAL(vl.vepIndexByName("Oranguta-Klaus", false), -1);
	}

	//test sort function for VCF files
	void sort()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/sort_in.vcf"));
		vl.checkValid();
		vl.sort();
		vl.store("out/sort_out.vcf");
		COMPARE_FILES("out/sort_out.vcf",TESTDATA("data_out/sort_out.vcf"));
		VCF_IS_VALID("out/sort_out.vcf")
	}

	//test sort function for TSV files
	void sort2()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/sort_in.tsv"));
		vl.checkValid();
		vl.sort();
		vl.store("out/sort_out.tsv");
		COMPARE_FILES("out/sort_out.tsv",TESTDATA("data_out/sort_out.tsv"));

	}

	//test sort function for VCF files (with quality)
	void sort3()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));
		vl.checkValid();
		vl.sort(true);
		//entries should be sorted numerically

		X_EQUAL(vl[0].chr() ,Chromosome("chr1"));
		I_EQUAL(vl[0].start(),11676308);
		I_EQUAL(vl[1].start(),11676377);
		X_EQUAL(vl[2].chr(), Chromosome("chr2"));
		I_EQUAL(vl[2].start(),139498511);
		X_EQUAL(vl[3].chr(), Chromosome("chr4"));
		I_EQUAL(vl[3].start(),68247038);
		I_EQUAL(vl[4].start(),68247113);
		X_EQUAL(vl[5].chr(), Chromosome("chr9"));
		I_EQUAL(vl[5].start(),130931421);
		I_EQUAL(vl[6].start(),130932396);
		X_EQUAL(vl[7].chr(), Chromosome("chr17"));
		I_EQUAL(vl[7].start(),72196817);
		I_EQUAL(vl[8].start(),72196887);
		I_EQUAL(vl[9].start(),72196892);
		X_EQUAL(vl[10].chr(), Chromosome("chr18"));
		I_EQUAL(vl[10].start(),67904549);
		I_EQUAL(vl[11].start(),67904586);
		I_EQUAL(vl[12].start(),67904672);
		X_EQUAL(vl[13].chr(), Chromosome("chr19"));
		I_EQUAL(vl[13].start(),14466629);
	}

	//test sortByFile function for *.vcf-files
	void sortByFile()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_snpeff.vcf"));
		vl.checkValid();
		vl.sortByFile(TESTDATA("data_in/variantList_sortbyFile.fai"));
		vl.store("out/sortByFile.vcf");
		//entries should be sorted by variantList_sortbyFile.fai, which is reverse-numeric concerning chromosomes
		VCF_IS_VALID("out/sortByFile.vcf")
		X_EQUAL(vl[0].chr(),Chromosome("chr19"));
		I_EQUAL(vl[0].start(),14466629);
		X_EQUAL(vl[1].chr(),Chromosome("chr18"));
		I_EQUAL(vl[1].start(),67904549);
		I_EQUAL(vl[2].start(),67904586);
		I_EQUAL(vl[3].start(),67904672);
		X_EQUAL(vl[4].chr(),Chromosome("chr17"));
		I_EQUAL(vl[4].start(),72196817);
		I_EQUAL(vl[5].start(),72196887);
		I_EQUAL(vl[6].start(),72196892);
		X_EQUAL(vl[7].chr(),Chromosome("chr9"));
		I_EQUAL(vl[7].start(),130931421);
		I_EQUAL(vl[8].start(),130932396);
		X_EQUAL(vl[9].chr(),Chromosome("chr4"));
		I_EQUAL(vl[9].start(),68247038);
		I_EQUAL(vl[10].start(),68247113);
		X_EQUAL(vl[11].chr(),Chromosome("chr2"));
		I_EQUAL(vl[11].start(),139498511);
		X_EQUAL(vl[12].chr() ,Chromosome("chr1"));
		I_EQUAL(vl[12].start(),11676308);
		I_EQUAL(vl[13].start(),11676377);
	}

	//test sortByFile function for *.tsv-files
	void sortByFile2()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/sort_in.tsv"));
		vl.checkValid();
		vl.sortByFile(TESTDATA("data_in/variantList_sortbyFile.fai"));
		vl.store("out/sortByFile_out.tsv");
		COMPARE_FILES("out/sortByFile_out.tsv",TESTDATA("data_out/sortByFile_out.tsv"));
	}

	void sortCustom()
	{
		VariantList vl;
		vl.checkValid();
		vl.load(TESTDATA("data_in/sort_in.vcf"));
		vl.sortCustom([](const Variant& a, const Variant& b) {return a.start() < b.start(); });

		I_EQUAL(vl.count(), 2344);
		X_EQUAL(vl[0].chr(),Chromosome("chr4"));
		I_EQUAL(vl[0].start(),85995);
		X_EQUAL(vl[1].chr(),Chromosome("chr4"));
		I_EQUAL(vl[1].start(),85997);
		X_EQUAL(vl[2].chr(),Chromosome("chr4"));
		I_EQUAL(vl[2].start(),86101);
		X_EQUAL(vl[3].chr(),Chromosome("chr4"));
		I_EQUAL(vl[3].start(),86102);
		X_EQUAL(vl[4].chr(),Chromosome("chr4"));
		I_EQUAL(vl[4].start(),87313);
		X_EQUAL(vl[5].chr(),Chromosome("chr20"));
		I_EQUAL(vl[5].start(),126309);
		X_EQUAL(vl[6].chr(),Chromosome("chr20"));
		I_EQUAL(vl[6].start(),126310);
		//...
		X_EQUAL(vl[2343].chr(),Chromosome("chr1"));
		I_EQUAL(vl[2343].start(),248802249);
	}


	void removeAnnotation()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		vl.checkValid();
		int index = vl.annotationIndexByName("1000g", true, false);

		I_EQUAL(vl.annotations().count(), 30);
		I_EQUAL(vl.count(), 329);
		I_EQUAL(vl[0].annotations().count(), 30);
		S_EQUAL(vl[0].annotations()[index-1], QByteArray("rs12569127"));
		S_EQUAL(vl[0].annotations()[index], QByteArray("0.1903"));
		S_EQUAL(vl[0].annotations()[index+1], QByteArray("0.2659"));

		vl.removeAnnotation(index);

		I_EQUAL(vl.annotations().count(), 29);
		I_EQUAL(vl.count(), 329);
		I_EQUAL(vl[0].annotations().count(), 29);
		S_EQUAL(vl[0].annotations()[index-1], QByteArray("rs12569127"));
		S_EQUAL(vl[0].annotations()[index], QByteArray("0.2659"));
		S_EQUAL(vl[0].annotations()[index+1], QByteArray(""));
	}

	//bug (number of variants was used to checked if index is out of range)
	void removeAnnotation_bug()
	{
		VariantList vl;
		vl.annotationDescriptions().append(VariantAnnotationDescription("bla", "some desciption"));
		vl.annotations().append(VariantAnnotationHeader("bla"));

		vl.removeAnnotation(0);

		I_EQUAL(vl.annotations().count(), 0)
	}

	void copyMetaData()
	{
		VariantList vl;
		vl.annotationDescriptions().append(VariantAnnotationDescription("bla", "some desciption"));
		vl.annotations().append(VariantAnnotationHeader("bla"));
		vl.filters().insert("MAF", "Minor allele frequency filter");
		vl.addCommentLine("Comment1");
		vl.append(Variant(Chromosome("chr1"), 1, 2, "A", "C"));

		//copy meta data
		VariantList vl2;
		vl2.copyMetaData(vl);

		//check meta data
		I_EQUAL(vl2.annotationDescriptions().count(), 1);
		I_EQUAL(vl2.annotations().count(), 1);
		I_EQUAL(vl2.filters().count(), 1);
		I_EQUAL(vl2.comments().count(), 1);

		//check no variants
		I_EQUAL(vl2.count(), 0);
	}

	void addAnnotation()
	{
		VariantList vl;
		vl.append(Variant(Chromosome("chr1"), 1, 2, "A", "C"));
		vl.append(Variant(Chromosome("chr2"), 1, 2, "A", "C"));

		int index = vl.addAnnotation("name", "desc", "default");

		I_EQUAL(index, 0);
		I_EQUAL(vl.annotations().count(), 1);
		S_EQUAL(vl.annotationDescriptionByName("name", false, true).description(), "desc");
		I_EQUAL(vl[0].annotations().count(), 1);
		S_EQUAL(vl[0].annotations()[index], "default");
		I_EQUAL(vl[1].annotations().count(), 1);
		S_EQUAL(vl[1].annotations()[index], "default");
	}

	void addAnnotationIfMissing()
	{
		VariantList vl;
		vl.append(Variant(Chromosome("chr1"), 1, 2, "A", "C"));
		vl.append(Variant(Chromosome("chr2"), 1, 2, "A", "C"));

		I_EQUAL(vl.addAnnotation("name", "desc", "default"), 0);
		S_EQUAL(vl.annotationDescriptionByName("name").description(), "desc");
		I_EQUAL(vl.addAnnotationIfMissing("name", "desc_new", "default"), 0);
		S_EQUAL(vl.annotationDescriptionByName("name").description(), "desc_new");
		I_EQUAL(vl.addAnnotationIfMissing("name2", "desc2", "default2"), 1);
		S_EQUAL(vl.annotationDescriptionByName("name2").description(), "desc2");
		I_EQUAL(vl.addAnnotationIfMissing("name2", "desc_new2", "default2"), 1);
		S_EQUAL(vl.annotationDescriptionByName("name2").description(), "desc_new2");
	}

	void removeAnnotationByName()
	{
		VariantList vl;
		vl.append(Variant(Chromosome("chr1"), 1, 2, "A", "C"));
		vl.append(Variant(Chromosome("chr2"), 1, 2, "A", "C"));
		vl.addAnnotation("name", "desc", "default");

		vl.removeAnnotationByName("name", true, true);

		I_EQUAL(vl.annotations().count(), 0);
		I_EQUAL(vl[0].annotations().count(), 0);
		I_EQUAL(vl[1].annotations().count(), 0);
	}

	void getSampleHeader_singlesample()
	{
		QString input = TESTDATA("data_in/panel_vep.GSvar");
		VariantList vl;
		vl.load(input);
		SampleHeaderInfo info = vl.getSampleHeader();
		I_EQUAL(info.count(), 1);
		I_EQUAL(info.sampleColumns(true).count(), 1);
		I_EQUAL(info.sampleColumns(true)[0], 0);
		I_EQUAL(info.sampleColumns(false).count(), 0);
	}

	void getSampleHeader_multisample()
	{
		QString input = TESTDATA("data_in/VariantFilter_in_multi.GSvar");
		VariantList vl;
		vl.load(input);
		SampleHeaderInfo info = vl.getSampleHeader();
		I_EQUAL(info.count(), 4);
		I_EQUAL(info.sampleColumns(true).count(), 2);
		I_EQUAL(info.sampleColumns(true)[0], 0);
		I_EQUAL(info.sampleColumns(true)[1], 3);
		I_EQUAL(info.sampleColumns(false).count(), 2);
		I_EQUAL(info.sampleColumns(false)[0], 1);
		I_EQUAL(info.sampleColumns(false)[1], 2);
	}

	void getPipeline()
	{
		VariantList vl;
		vl.load(TESTDATA("data_in/panel_vep.GSvar"));
		S_EQUAL(vl.getPipeline(), "megSAP 0.1-742-ged8ba02");

		//header not set
		vl.load(TESTDATA("data_in/VariantFilter_in_multi.GSvar"));
		S_EQUAL(vl.getPipeline(), "n/a");
	}
};
