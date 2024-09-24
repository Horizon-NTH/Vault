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

// TODO: Application test refactoring
// TEST_F(ApplicationTest, OpenVault)
// {
// 	const char* args[] = {"program", "open", "-v/path/to/vault", "-d/output/path"};
// 	init(std::span(args));
// 	EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
// }
//
// TEST_F(ApplicationTest, CloseVault)
// {
// 	const char* args[] = {"program", "close", "-v/path/to/vault", "-d/output/path", "-e.vlt"};
// 	init(std::span(args));
// 	EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
// }
//
// TEST_F(ApplicationTest, EmptyCommandResultAsHelp)
// {
// 	const char* args[] = {"program"};
// 	init(std::span(args));
// 	EXPECT_EQ(m_app->execute(), EXIT_SUCCESS);
// }
//
// TEST_F(ApplicationTest, OpenVaultAttributeRequired)
// {
// 	const char* args[] = {"program", "open", "-d/output/path"};
// 	init(std::span(args));
// 	EXPECT_EQ(m_app->execute(), EXIT_FAILURE);
// }
//
// TEST_F(ApplicationTest, CloseVaultAttributeRequired)
// {
// 	const char* args[] = {"program", "close", "-d/output/path", "-e", ".vlt"};
// 	init(std::span(args));
// 	EXPECT_EQ(m_app->execute(), EXIT_FAILURE);
// }
