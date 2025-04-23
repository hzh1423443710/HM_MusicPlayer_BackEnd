#include "EmailUtil.h"
#include "Config.h"

#include <curl/curl.h>
#include <curl/easy.h>
#include <spdlog/spdlog.h>

#include <random>
#include <sstream>
#include <string>

constexpr const char* TAG = "[EmailUtil]";

bool EmailUtil::sendTextEmail(const std::string& to, const std::string& subject,
							  const std::string& body) {
	const auto& config = Config::getInstance()->getVerifyServiceConfig();

	CURL* curl = curl_easy_init();
	if (curl == nullptr) {
		spdlog::error("{} Failed to initialize curl", TAG);
		return false;
	}

	std::ostringstream oss;
	oss << "To: " << to << "\r\n"
		<< "From: " << config.email_from << "\r\n"
		<< "Subject: " << subject << "\r\n\r\n"
		<< body;
	std::string content = oss.str();

	EmailData email_data{};
	email_data.data = content.c_str();
	email_data.size_left = content.size();

	curl_slist* recipients = nullptr;
	recipients = curl_slist_append(recipients, to.c_str());

	// 设置CURL选项
	curl_easy_setopt(curl, CURLOPT_URL, config.smtp_server_url.c_str());
	curl_easy_setopt(curl, CURLOPT_USERNAME, config.smtp_user.c_str());
	curl_easy_setopt(curl, CURLOPT_PASSWORD, config.smtp_password.c_str());
	curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
	curl_easy_setopt(curl, CURLOPT_MAIL_FROM, config.email_from.c_str());
	curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
	curl_easy_setopt(curl, CURLOPT_READFUNCTION, readCallback);
	curl_easy_setopt(curl, CURLOPT_READDATA, &email_data);
	curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
	// curl_easy_setopt(m_curl, CURLOPT_VERBOSE, 1L);

	// 发送
	CURLcode res = curl_easy_perform(curl);
	if (res != CURLE_OK) {
		spdlog::error("{} Failed to send email: {}", TAG, curl_easy_strerror(res));
		curl_slist_free_all(recipients);
		return false;
	}

	// 清理
	curl_slist_free_all(recipients);
	curl_easy_cleanup(curl);

	spdlog::info("{} Email sent successfully to {}", TAG, to);

	return true;
}

std::string EmailUtil::generateVerificationCode(int length) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(0, 9);
	std::string code;
	for (int i = 0; i < length; ++i) {
		code += std::to_string(dis(gen));
	}

	return code;
}

size_t EmailUtil::readCallback(void* ptr, size_t size, size_t nmemb, void* data) {
	EmailData* email_data = static_cast<EmailData*>(data);
	size_t buffer_size = size * nmemb;
	if (email_data->size_left == 0) {
		return 0;
	}

	size_t copy_size = std::min(buffer_size, email_data->size_left);
	memcpy(ptr, email_data->data, copy_size);
	email_data->data += copy_size;
	email_data->size_left -= copy_size;

	return copy_size;
}
