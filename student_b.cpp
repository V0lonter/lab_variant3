#include "shared_types.h"

// TODO (Студент Б): кубічна сплайн-інтерполяція
std::unique_ptr<Result> calculateB(std::shared_ptr<const InputData> data) {
    (void)data;
    return std::make_unique<Result>(Result{"Spline (stub)", 0.0, 0.0, 0, 0.0});
}
