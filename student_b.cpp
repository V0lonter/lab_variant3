// Студент Б — кубічна сплайн-інтерполяція (природний сплайн: S''(x_0) = S''(x_n) = 0).
#include <algorithm>
#include <chrono>

#include "shared_types.h"

namespace {

// Другі похідні M_i у вузлах. Для внутрішніх вузлів:
//   h_{i-1} M_{i-1} + 2 (h_{i-1} + h_i) M_i + h_i M_{i+1}
//       = 6 [ (y_{i+1} - y_i)/h_i - (y_i - y_{i-1})/h_{i-1} ],   M_0 = M_n = 0.
// Тридіагональну систему розв'язуємо методом прогонки (Томаса).
std::vector<double> second_derivatives(const std::vector<Point>& p, long long& ops) {
    const std::size_t n = p.size();
    std::vector<double> m(n, 0.0);
    if (n < 3) {
        return m;  // два вузли -> сплайн вироджується в пряму
    }

    std::vector<double> h(n - 1);
    for (std::size_t i = 0; i + 1 < n; ++i) {
        h[i] = p[i + 1].x - p[i].x;
        ops += 1;
    }

    const std::size_t k = n - 2;  // кількість невідомих M_1 .. M_{n-2}
    std::vector<double> diag(k), rhs(k);
    for (std::size_t r = 0; r < k; ++r) {  // рядок r відповідає вузлу i = r + 1
        diag[r] = 2.0 * (h[r] + h[r + 1]);                                         // 2 операції
        rhs[r] = 6.0 * ((p[r + 2].y - p[r + 1].y) / h[r + 1]                       // 6 операцій
                        - (p[r + 1].y - p[r].y) / h[r]);
        ops += 8;
    }

    // Прямий хід прогонки: піддіагональ рядка r дорівнює h[r], наддіагональ — h[r + 1].
    for (std::size_t r = 1; r < k; ++r) {
        const double w = h[r] / diag[r - 1];
        diag[r] -= w * h[r];         // наддіагональ попереднього рядка = h[r]
        rhs[r] -= w * rhs[r - 1];
        ops += 5;  // 1 ділення + 2 + 2
    }

    // Зворотний хід.
    std::vector<double> sol(k);
    sol[k - 1] = rhs[k - 1] / diag[k - 1];
    ops += 1;
    for (std::size_t r = k - 1; r-- > 0;) {
        sol[r] = (rhs[r] - h[r + 1] * sol[r + 1]) / diag[r];
        ops += 3;
    }

    for (std::size_t r = 0; r < k; ++r) {
        m[r + 1] = sol[r];
    }
    return m;
}

// Значення сплайна в точці x: бінарний пошук інтервала + кубічний многочлен на ньому.
double spline_value(const std::vector<Point>& p, const std::vector<double>& m, double x, long long& ops) {
    auto it = std::upper_bound(p.begin(), p.end(), x,
                               [](double v, const Point& q) { return v < q.x; });
    std::size_t i = (it == p.begin()) ? 0 : static_cast<std::size_t>(it - p.begin()) - 1;
    if (i + 1 >= p.size()) {
        i = p.size() - 2;  // x збігається з правим кінцем таблиці
    }

    const double h = p[i + 1].x - p[i].x;
    const double a = p[i + 1].x - x;
    const double b = x - p[i].x;
    const double h6 = 6.0 * h;

    const double result = m[i] * a * a * a / h6 + m[i + 1] * b * b * b / h6
                        + (p[i].y / h - m[i] * h / 6.0) * a
                        + (p[i + 1].y / h - m[i + 1] * h / 6.0) * b;
    ops += 25;  // 3 (h, a, b) + 1 (6h) + 4 + 4 + 5 + 5 + 3 (додавання)
    return result;
}

double spline_at(const std::vector<Point>& p, double x, long long& ops) {
    return spline_value(p, second_derivatives(p, ops), x, ops);
}

}  // namespace

std::unique_ptr<Result> calculateB(std::shared_ptr<const InputData> data) {
    validate_input(data);
    const std::vector<Point>& pts = data->points;
    const double x0 = data->x0;

    // --- основне обчислення: побудова сплайна + значення в x0 ---
    long long ops = 0;
    const auto t0 = std::chrono::steady_clock::now();
    const double value = spline_at(pts, x0, ops);
    const auto t1 = std::chrono::steady_clock::now();
    const double time_us = std::chrono::duration<double, std::micro>(t1 - t0).count();

    // --- оцінка похибки ---
    // Так само, як у методі Лагранжа: прибираємо найближчий до x0 внутрішній вузол
    // і будуємо сплайн заново. Похибка кубічного сплайна ~ h^4, а після видалення вузла
    // локальний крок подвоюється, тобто похибка зростає приблизно в 2^4 = 16 разів.
    // Тому |S - S_reduced| ~ 15 * (похибка S), звідси ділення на 15 (ідея Річардсона).
    // Це евристика, а не гарантована межа.
    std::vector<Point> reduced = pts;
    reduced.erase(reduced.begin() + static_cast<std::ptrdiff_t>(nearest_interior_index(pts, x0)));
    long long unused_ops = 0;  // вартість оцінки в загальний підрахунок не входить
    const double error_estimate = std::abs(value - spline_at(reduced, x0, unused_ops)) / 15.0;

    return std::make_unique<Result>(Result{"Кубічний сплайн", value, error_estimate, ops, time_us});
}
