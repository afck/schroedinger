#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestCaller.h>
#include <cppunit/TestFixture.h>
#include <cppunit/ui/text/TestRunner.h>

#include "Field.h"

class FieldTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FieldTest);
  CPPUNIT_TEST(testWrap);
  CPPUNIT_TEST(testMirror);
  CPPUNIT_TEST(testZero);
  CPPUNIT_TEST_SUITE_END();

public:
  void testWrap();
  void testMirror();
  void testZero();

private:
  const int width = 5;
  const int height = 3;
  const int border = 2;
};

void FieldTest::testWrap() {
  Field<int> field(width, height, border, WRAP);
  field.set(1, 2, 300);
  field.set(4, 0, 500);
  CPPUNIT_ASSERT_EQUAL(0, field.safeGet(1, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.safeGet(6, 2));
  CPPUNIT_ASSERT_EQUAL(500, field.safeGet(4, 0));
  CPPUNIT_ASSERT_EQUAL(500, field.safeGet(-1, 0));
  CPPUNIT_ASSERT_EQUAL(500, field.safeGet(-1, 3));
  CPPUNIT_ASSERT_EQUAL(500, field.safeGet(4, 3));
  field.fillBorder();
  CPPUNIT_ASSERT_EQUAL(0, field.get(1, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.get(1, 2));
  CPPUNIT_ASSERT_EQUAL(300, field.get(6, 2));
  CPPUNIT_ASSERT_EQUAL(500, field.get(4, 0));
  CPPUNIT_ASSERT_EQUAL(500, field.get(-1, 0));
  CPPUNIT_ASSERT_EQUAL(500, field.get(-1, 3));
  CPPUNIT_ASSERT_EQUAL(500, field.get(4, 3));
}

void FieldTest::testMirror() {
  Field<int> field(width, height, border, MIRROR);
  field.set(1, 2, 300);
  field.set(4, 0, 500);
  CPPUNIT_ASSERT_EQUAL(0, field.safeGet(2, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.safeGet(1, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.safeGet(1, 2));
  CPPUNIT_ASSERT_EQUAL(300, field.safeGet(-2, 2));
  CPPUNIT_ASSERT_EQUAL(500, field.safeGet(5, 0));
  CPPUNIT_ASSERT_EQUAL(500, field.safeGet(5, -1));
  field.fillBorder();
  CPPUNIT_ASSERT_EQUAL(0, field.get(2, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.get(1, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.get(1, 2));
  CPPUNIT_ASSERT_EQUAL(300, field.get(-2, 2));
  CPPUNIT_ASSERT_EQUAL(500, field.get(5, 0));
  CPPUNIT_ASSERT_EQUAL(500, field.get(5, -1));
}

void FieldTest::testZero() {
  Field<int> field(width, height, border, ZERO);
  field.set(1, 2, 300);
  field.set(4, 0, 500);
  CPPUNIT_ASSERT_EQUAL(0, field.safeGet(1, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.safeGet(1, 2));
  CPPUNIT_ASSERT_EQUAL(0, field.safeGet(6, 2));
  CPPUNIT_ASSERT_EQUAL(500, field.safeGet(4, 0));
  CPPUNIT_ASSERT_EQUAL(0, field.safeGet(-1, 0));
  CPPUNIT_ASSERT_EQUAL(0, field.safeGet(-1, 3));
  CPPUNIT_ASSERT_EQUAL(0, field.safeGet(4, 3));
  field.fillBorder();
  CPPUNIT_ASSERT_EQUAL(0, field.get(1, 3));
  CPPUNIT_ASSERT_EQUAL(300, field.get(1, 2));
  CPPUNIT_ASSERT_EQUAL(0, field.get(6, 2));
  CPPUNIT_ASSERT_EQUAL(500, field.get(4, 0));
  CPPUNIT_ASSERT_EQUAL(0, field.get(-1, 0));
  CPPUNIT_ASSERT_EQUAL(0, field.get(-1, 3));
  CPPUNIT_ASSERT_EQUAL(0, field.get(4, 3));
}

int main(int argc, char **argv) {
  CppUnit::TestSuite *suite = new CppUnit::TestSuite("Schroedinger Tests");
  suite->addTest(FieldTest::suite());
  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);
  runner.run();
  return 0;
}
