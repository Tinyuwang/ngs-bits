#include "../TestFramework.h"

class BamIndex_Test
		: public QObject
{
	Q_OBJECT

private slots:
	
	void test_01()
	{
#ifdef WIN32
		QSKIP("Creating indices does not work (only Windows, mingw 4.91)");
#endif

	    QFile::copy(QFINDTESTDATA("../cppNGS-TEST/data_in/panel.bam"), "out/BamIndex.bam");
	    TFW_EXEC("BamIndex", "-in out/BamIndex.bam");
	    QVERIFY(QFile::exists("out/BamIndex.bam.bai"));
	}
};

TFW_DECLARE(BamIndex_Test)


