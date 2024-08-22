#include "Application.h"

#include <gtest/gtest.h>

class ApplicationTest : public testing::Test
{
protected:
	std::unique_ptr<Application> m_app;

	void init(const std::span<const char*>& args)
	{
		m_app = std::make_unique<Application>(args);
	}
};

TEST_F(ApplicationTest, OpenVault)
{
	const char* args[] = {"program", "open", "-v/path/to/vault", "-d/output/path"};
	EXPECT_NO_THROW(init(std::span(args)));
}

TEST_F(ApplicationTest, CloseVault)
{
	const char* args[] = {"program", "close", "-v/path/to/vault", "-d/output/path", "-e.vlt"};
	EXPECT_NO_THROW(init(std::span(args)));
}

TEST_F(ApplicationTest, EmptyCommandResultAsHelp)
{
	const char* args[] = {"program"};
	EXPECT_NO_THROW(init(std::span(args)));
}

TEST_F(ApplicationTest, OpenVaultAttributeRequired)
{
	const char* args[] = {"program", "open", "-d/output/path"};
	EXPECT_THROW(init(std::span(args)), std::runtime_error);
}

TEST_F(ApplicationTest, CloseVaultAttributeRequired)
{
	const char* args[] = {"program", "close", "-d/output/path", "-e", ".vlt"};
	EXPECT_THROW(init(std::span(args)), std::runtime_error);
}
