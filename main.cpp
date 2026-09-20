#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "shared_types.h"

namespace {

double f_sin(double x) { return std::sin(x); }

// Функція Рунге: на рівномірних вузлах поліном Лагранжа "розгойдується" біля кінців.
double f_runge(double x) { return 1.0 / (1.0 + 25.0 * x * x); }

// Таблиця значень функції f у n рівновіддалених вузлах на [a, b].
std::shared_ptr<const InputData> make_input(double (*f)(double), double a, double b, int n, double x0) {
    std::vector<Point> pts;
    pts.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const double x = a + (b - a) * i / (n - 1);
        pts.push_back({x, f(x)});
    }
    return std::make_shared<const InputData>(InputData{std::move(pts), x0});
}

// --- форматування таблиці (вирівнювання з урахуванням кирилиці в UTF-8) ---
std::size_t utf8_len(const std::string& s) {
    std::size_t n = 0;
    for (unsigned char c : s) {
        if ((c & 0xC0) != 0x80) {
            ++n;
        }
    }
    return n;
}

std::string pad(const std::string& s, std::size_t width) {
    const std::size_t len = utf8_len(s);
    return len >= width ? s : s + std::string(width - len, ' ');
}

std::string num(double v, int precision, bool scientific = false) {
    std::ostringstream os;
    os << (scientific ? std::scientific : std::fixed) << std::setprecision(precision) << v;
    return os.str();
}

void print_header() {
    std::cout << pad("Метод", 18) << pad("y(x0)", 14) << pad("Оцінка похибки", 16)
              << pad("Факт. похибка", 15) << pad("Операцій", 10) << "Час, мкс\n"
              << std::string(84, '-') << '\n';
}

void print_row(const std::string& method, double value, double error_estimate,
               double true_error, long long ops, double time_us) {
    std::cout << pad(method, 18) << pad(num(value, 8), 14) << pad(num(error_estimate, 2, true), 16)
              << pad(num(true_error, 2, true), 15) << pad(std::to_string(ops), 10)
              << num(time_us, 2) << '\n';
}

}  // namespace

int main() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);  // щоб кирилиця коректно виводилась у консолі Windows
#endif

    try {
        // Єдиний спільний набір вхідних даних для обох алгоритмів.
        const double x0 = 1.1;
        const auto data = make_input(f_sin, 0.0, 3.0, 8, x0);
        const double exact = f_sin(x0);

        std::cout << "Інтерполяція таблично заданої функції y = sin(x)\n"
                  << "Вузлів: " << data->points.size() << " (рівномірно на [0, 3]), x0 = " << x0
                  << ", точне значення = " << num(exact, 8) << "\n\n";
        print_header();

        // ===== ДІЛЯНКА ВИКЛИКІВ АЛГОРИТМІВ =====
        // Сюди додають виклики своїх алгоритмів Студент А та Студент Б.
        // ===== КІНЕЦЬ ДІЛЯНКИ =====
    } catch (const std::exception& e) {
        std::cerr << "Помилка: " << e.what() << '\n';
        return 1;
    }
    return 0;
}
