#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
#include <stdexcept>
#include <yaml-cpp/yaml.h>
#include <webdriverxx/webdriver.h>
#include <webdriverxx/browsers/chrome.h>

using namespace std;
using namespace std::chrono;
using namespace webdriverxx;

// 최대 timeout_sec 초 동안 500ms 간격으로 Locator가 나타날 때까지 대기
template<typename Locator>
void WaitForElement(WebDriver& driver, const Locator& by, int timeout_sec = 15) {
    auto start = steady_clock::now();
    while (true) {
        try {
            driver.FindElement(by);
            return;
        } catch (...) {
            if (steady_clock::now() - start > seconds(timeout_sec))
                throw runtime_error("Timeout waiting for element");
            this_thread::sleep_for(milliseconds(500));
        }
    }
}

// 오늘 H:M:S 시각의 system_clock::time_point 생성
system_clock::time_point TodayAt(int H, int M, int S) {
    auto now = system_clock::now();
    time_t tt = system_clock::to_time_t(now);
    tm local = *localtime(&tt);
    local.tm_hour = H;
    local.tm_min  = M;
    local.tm_sec  = S;
    local.tm_isdst = -1;
    return system_clock::from_time_t(mktime(&local));
}

int main() {
    try {
        // 1) 설정 YAML 로드
        auto cfg = YAML::LoadFile("config.yaml");
        string prodId   = cfg["prodId"].as<string>();
        string playDate = cfg["playDate"].as<string>();
        string playSeq  = cfg["playSeq"].as<string>();
        string preSale  = cfg["preSale"].as<string>();
        int H = cfg["Hour"].as<int>();
        int M = cfg["Minute"].as<int>();
        int S = cfg["Second"].as<int>();

        // 2) WebDriver 시작 (chromedriver 자동 실행)
        Chrome opts;
        WebDriver driver = Start(opts, "http://localhost:9515");

        // 3) nol.interpark.com 으로 이동 → 로그인 버튼 클릭
        driver.Navigate("https://nol.interpark.com/");
        By loginButton = ByXPath("//button[.//li[contains(normalize-space(),'로그인')]]");
        WaitForElement(driver, loginButton, 10);
        driver.FindElement(loginButton).Click();
        cout << "[2] 로그인 버튼 클릭 완료\n";

        // 4) 수동 로그인 안내 alert + 90초 대기
        driver.Execute(R"(
            alert("⚠️ 이제 로그인을 수동으로 진행해 주세요.\n90초 동안 기다립니다.");
        )");
        this_thread::sleep_for(seconds(90));

        // 5) 정확한 호출 시각 계산
        auto target = TodayAt(H, M, S);

        // 6) 목표 1초 전까지 대기
        while (system_clock::now() < target - seconds(1)) {
            this_thread::sleep_for(milliseconds(100));
        }

        // 7) 1초 남았을 때부터 나노초 단위까지 같은 줄 카운트다운
        while (true) {
            auto now = system_clock::now();
            if (now >= target) break;
            auto diff_ns = duration_cast<nanoseconds>(target - now).count();
            // ms 부분 (0~999)
            int ms = (diff_ns / 1'000'000) % 1000;
            // micro+ns 부분 (0~999999)
            int frac = diff_ns % 1'000'000;
            // 출력: "0.mmm.ffffff 남음"
            cout << '\r'
                 << "⏳ 0."
                 << setw(3) << setfill('0') << ms << "."
                 << setw(6) << setfill('0') << frac
                 << " 남음" << flush;
        }
        cout << "\n[3] 정확한 순간! 호출 실행...\n";

        // 8) JS fetch 호출 & 팝업 오픈
        ostringstream js;
        js << R"(
            (async function() {
                try {
                  const resp = await fetch(
                    'https://api-ticketfront.interpark.com/v1/goods/)"
                    << prodId
                    << R"(/waiting?channelCode=PC&preSales=)"
                    << preSale
                    << R"(&playDate=)"
                    << playDate
                    << R"(&playSeq=)"
                    << playSeq
                    << R"(',
                    { credentials: 'include' }
                  );
                  const d = (await resp.json()).data;
                  if (d && d !== 'NP' && d !== 'N') {
                    window.open(d);
                  } else {
                    console.log('대기 중:', d);
                  }
                } catch (e) {
                  console.error('API 에러:', e);
                }
            })();
        )";
        driver.Execute(js.str());
        cout << "[4] 호출 완료: "
             << setw(2) << setfill('0') << H << ":"
             << setw(2) << M << ":"
             << setw(2) << S << "\n";
    }
    catch (const exception& ex) {
        cerr << "❌ 예외 발생: " << ex.what() << endl;
        return 1;
    }
    return 0;
}