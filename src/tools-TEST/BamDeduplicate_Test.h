#include "TestFramework.h"
#include "QFileInfo"
#include <stdlib.h>

TEST_CLASS(BamDeduplicate_Test)
{
Q_OBJECT
private slots:

	void test_bam_without_duplicates()
	{
		EXECUTE("BamDeduplicate", "-bam " + TESTDATA("data_in/BamDeduplicate_in.bam") + " -index " + TESTDATA("data_in/BamDeduplicate_in.fastq.gz") + " -out out/BamDeduplicate_out.bam");
		IS_TRUE(QFile::exists("out/BamDeduplicate_out.bam"));
	}

	void test_bam_with_many_duplicates_same_barcode()
	{
		EXECUTE("BamDeduplicate", "-bam " + TESTDATA("data_in/BamDeduplicate_in2.bam") + " -index " + TESTDATA("data_in/BamDeduplicate_in2.fastq") + " -out out/BamDeduplicate_out2.bam");
		IS_TRUE(QFile::exists("out/BamDeduplicate_out2.bam"));
	}

	void test_bam_with_many_duplicates_and_unique_barcodes()
	{
		EXECUTE("BamDeduplicate", "-bam " + TESTDATA("data_in/BamDeduplicate_in2.bam") + " -index " + TESTDATA("data_in/BamDeduplicate_in3.fastq") + " -out out/BamDeduplicate_out3.bam");
		IS_TRUE(QFile::exists("out/BamDeduplicate_out2.bam"));
	}

	void test_compare_deduplicated_with_same_barcode_to_unique_barcodes()
	{
		QFileInfo same(TESTDATA("data_in/BamDeduplicate_in2.bam"));
		QFileInfo unique("out/BamDeduplicate_out3.bam");
		IS_TRUE((unique.size()-same.size())>8000);
	}

	void test_duplicates_regarding_mip_file()
	{
		EXECUTE("BamDeduplicate", "-bam " + TESTDATA("data_in/BamDeduplicateByBarcode_in4.bam") + " -index " + TESTDATA("data_in/BamDeduplicateByBarcode_index_in4.fastq.gz") + " -mip_file " + TESTDATA("data_in/FastqBarcodeToHeader_in_mips1.txt") +" -out out/BamDeduplicate_out4.bam -mip_count_out out/BamDeduplicate_out1.tsv");
		COMPARE_FILES("out/BamDeduplicate_out1.tsv", TESTDATA("data_out/BamDeduplicate_out1.tsv"));
	}

};
