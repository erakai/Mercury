#include <QTest>

class MercuryTest : public QObject
{
  Q_OBJECT;

private:
  bool myCondition() { return true; }

private slots:
  /*
  void initTestCase() {
    qDebug("Called before everything else.");
  }
  */

  void example_test()
  {
    QVERIFY(true);  // check that a condition is satisfied
    QCOMPARE(1, 1); // compare two values
    QVERIFY(myCondition());
    QVERIFY(1 != 2);
  }

  /*
  void cleanupTestCase() {
    qDebug("Called after example_test.");
  }
  */
};

QTEST_MAIN(MercuryTest);
#include "mercury_test.moc"
