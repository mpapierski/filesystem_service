#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <fstream>
#include "filesystem_service/filesystem_service.hpp"

struct client_handlers
{
	MOCK_METHOD1(handle_rename, void(const boost::system::error_code &));
};

struct OperationsTest : ::testing::Test
{
	OperationsTest()
		: timeout_timer(io_service_)
		, fs_svc(io_service_)
	{
	}
	void SetUp()
	{
		timeout_timer.expires_from_now(boost::posix_time::seconds(5));
		timeout_timer.async_wait(boost::bind(&boost::asio::io_service::stop, &io_service_));
		std::ofstream ofs("testfile1.txt");
		ofs << "hello world";
	}
	void TearDown()
	{
		boost::system::error_code ec;
		boost::filesystem::remove("testfile1.txt", ec);
	}
	client_handlers client;
	boost::asio::io_service io_service_;
	boost::asio::deadline_timer timeout_timer;
	services::filesystem_service fs_svc;
};

using ::testing::SaveArg;
using ::testing::_;
using ::testing::Invoke;
using ::testing::DoAll;

#define SAVE_AND_STOP(ec) \
	DoAll( \
		SaveArg<0>(&(ec)), \
		Invoke(boost::bind(&boost::asio::io_service::stop, &io_service_)))
TEST_F (OperationsTest, AsyncRenameTestSuccess)
{
	boost::system::error_code ec;
	EXPECT_CALL(client, handle_rename(_))
		.WillOnce(SAVE_AND_STOP(ec));
	fs_svc.async_rename("testfile1.txt", "testfile2.txt",
		boost::bind(&client_handlers::handle_rename, &client, _1));
	io_service_.run();
	EXPECT_FALSE(ec);
}

TEST_F (OperationsTest, AsyncRenameTestFailure)
{
	boost::system::error_code ec;
	EXPECT_CALL(client, handle_rename(_))
		.WillOnce(SAVE_AND_STOP(ec));
	fs_svc.async_rename("not_existing_file.txt", "i_dont_know.txt",
		boost::bind(&client_handlers::handle_rename, &client, _1));
	io_service_.run();
	EXPECT_TRUE(ec);
}
