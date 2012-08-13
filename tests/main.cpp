#include <QtCore/QString>
#include <QtCore/QDate>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

void PrintTo(const QString& string, ::std::ostream* os) {
    *os<< "\"" << string.toStdString() << "\"";
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleMock(&argc, argv);
    return RUN_ALL_TESTS();
}
