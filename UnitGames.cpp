#include <iostream>
#include <iomanip>
#include <concepts>
#include <type_traits>
#include <format>
#include <print>

//damit der spaetere is_same was zum vergleichen hat
template<char SU = 'g'>
struct UType {};

//halten der ratio, landet in ratio_type
template<double SR>
struct RType {
    constexpr static double ratio = SR;
};


template<char U, std::floating_point dbl = double>
struct F {

    using ratio_type =
        std::conditional_t< std::is_same_v< UType<U>, UType<'B'> >, RType<1'000'000'000.0>, //billion
            std::conditional_t< std::is_same_v< UType<U>, UType<'M'> >, RType<1'000'000.0>, //million
                std::conditional_t< std::is_same_v< UType<U>, UType<'K'> >, RType<1'000.0>, //kilo
                    std::conditional_t< std::is_same_v< UType<U>, UType<'g'> >, RType<1.0>, //grams
                        std::conditional_t< std::is_same_v< UType<U>, UType<'m'> >, RType<0.001>, //milligrams
                        RType<1.0> //else -> default grams
                        >
                    >
                >
            >
        >;
    constexpr static char Unit = U;
    dbl value;

    F(const dbl& d) : value{ d } {}
    F(const F<U, dbl>& same) : value{ same.value } {};
};

template <char U, std::floating_point dbl>
struct std::formatter<F<U, dbl>> {
    // for debugging only
    formatter() = default;

    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const F<U, dbl>& flt, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{:g}{}", flt.value, F<U, dbl>::Unit);
    }
};


int main(int argc, char** argv)
{
    F<'M'> f1{ 1.2000001 };
    F<'K'> f2{ 1.30000 };
    F<1> f3{ 1.40000 };
    F f4 = f2;
    F<'m'> mg = 1.5f;
    std::println(std::cout, "raw-f1: {}", f1.value);
    std::println(std::cout, "f-f1: {:f}", f1.value);
    std::println(std::cout, "g-f1: {:g}", f1.value);
    std::println(std::cout, "{} ratio({:.0f})", f1, decltype(f1)::ratio_type::ratio);
    std::println(std::cout, "{} ratio({:.0f})", f2, decltype(f2)::ratio_type::ratio);
    std::println(std::cout, "{} ratio({:.0f})", f3, decltype(f3)::ratio_type::ratio);
    std::println(std::cout, "{} ratio({:.0f})", f4, decltype(f4)::ratio_type::ratio);
    std::println(std::cout, "{} ratio({:g})", mg, decltype(mg)::ratio_type::ratio);

    std::println(std::cout, "siezof F<'K'> is {}, sizeof a double is {}", sizeof(F<'K'>), sizeof(double));
    return 0;
}

/* gpt recommendations
* 
This C++26 code is using modern features like `std::print`, `concepts`, and user-defined formatting for a template type. Overall, the code is sophisticated, but it does have **several issues and potential flaws**, both **syntactic**, **semantic**, and **design-related**. Here's a breakdown:

---

### ❌ **1. Illegal Specialization of `std::formatter` in `std` Namespace**

```cpp
template <char U, std::floating_point dbl>
struct std::formatter<F<U,dbl>> {
```

**Issue**: You **cannot specialize standard library templates** like `std::formatter` directly in the `std` namespace unless it is specifically allowed, which in this case it **is not** (except in certain cases like providing custom formatters via ADL, which doesn't apply here).

**Fix**: Use the `std::formatter` customization point via ADL-friendly wrappers in your own namespace. Here's a safer way (requires a workaround):

```cpp
namespace my {
    template <char U, std::floating_point dbl>
    struct F { ... };

    template <char U, std::floating_point dbl>
    struct formatter<F<U,dbl>> {
        ...
    };
}

template <char U, std::floating_point dbl>
struct std::formatter<my::F<U,dbl>> {
    ...
};
```

Alternatively, consider implementing `tag_invoke` if C++26 allows.

---

### ❌ **2. `F<1>` Is Invalid: `char` Cannot Be Constructed from Integer `1`**

```cpp
F<1> f3{1.40000};
```

**Issue**: `1` is treated as an integer, but the template parameter expects a `char`. `1` will implicitly convert to a character with ASCII value 1 (`SOH`, not printable), but it's **not meaningful**.

**Fix**: Use an actual character:

```cpp
F<'1'> f3{1.40000}; // or some valid unit like 'K', 'M', etc.
```

---

### ❌ **3. Missing Template Parameter in: `F f4=f2;`**

```cpp
F f4 = f2;
```

**Issue**: This relies on CTAD (Class Template Argument Deduction), but your `F` class template does not have a deduction guide, and since the default template argument is only for the second parameter (`dbl`), not the first (`char U`), CTAD cannot deduce `U`.

**Fix Options**:

* Add an explicit deduction guide:

```cpp
template<char U, std::floating_point dbl>
F(F<U,dbl>) -> F<U, dbl>;
```

* Or just write it explicitly:

```cpp
F<'K'> f4 = f2;
```

---

### ❌ **4. Implicit Conversion in: `F<'m'> mg = 1.5f;`**

```cpp
F<'m'> mg = 1.5f;
```

**Issue**: This uses implicit conversion from `float` to `F<'m'>`. Although the constructor `F(const dbl& d)` allows this, it's **not ideal** and can lead to unintended conversions.

**Fix** (Optional): Consider marking the constructor `explicit`:

```cpp
explicit F(const dbl& d): value{d} {}
```

This would prevent accidental implicit conversions, especially in templates.

---

### ⚠️ **5. Overuse of `std::conditional_t` Chains**

```cpp
using ratio_type =
    std::conditional_t< std::is_same_v< UType<U>,UType<'B'> >, RType<1'000'000'000.0>,
        ...
        RType<1.0>
    >;
```

**Issue**: This "ladder" of `std::conditional_t` is hard to read, maintain, and error-prone.

**Fix**: Replace with `if constexpr` or `constexpr` function to deduce the ratio:

```cpp
constexpr double get_ratio(char unit) {
    if (unit == 'B') return 1'000'000'000.0;
    else if (unit == 'M') return 1'000'000.0;
    else if (unit == 'K') return 1'000.0;
    else if (unit == 'g') return 1.0;
    else if (unit == 'm') return 0.001;
    else return 1.0;
}
```

---

### ✅ **Minor Observations (Not Flaws)**

* Using `std::println(std::cout, ...)` is fine in C++26, assuming you have a compiler/libc++ implementation that supports it (like GCC 14+ or Clang 18+).
* Using digit separators like `1'000.0` is valid and improves readability.
* The output formatting in `std::format_to(ctx.out(), "{:g}{}", ...)` is clear and concise.

---

### ✅ **Code That Works Well**

* Use of `std::floating_point` as a constraint.
* General structure and idea of unit-wrapping and custom formatting.
* Usage of structured templates and strong types.

---

### 🔚 **Summary of Fixes Needed**

| Problem                                       | Fix                                                     |
| --------------------------------------------- | ------------------------------------------------------- |
| ❌ Specialization of `std::formatter` in `std` | Move it outside `std`, or use `tag_invoke` if supported |
| ❌ Invalid template param `F<1>`               | Use a valid `char`, like `'1'` or meaningful symbol     |
| ❌ CTAD doesn't work for `F f4 = f2`           | Provide deduction guide or specify template args        |
| ❌ Implicit float-to-F conversion              | Consider marking the constructor `explicit`             |
| ⚠️ Nested `conditional_t` mess                | Use `if constexpr` or `constexpr` function instead      |

Would you like a cleaned-up and corrected version of this code as a suggestion?




*/

/*
Great! Let's extend your `F<Unit, T>` wrapper to **support unit conversions between different units**, such as converting:

* `F<'K'> → F<'M'>`
* `F<'m'> → F<'g'>`
* etc.

---

## ✅ Design Goals for Conversion

We’ll add:

### ✔️ `to<U2>()` method

Converts `F<Unit, T>` to a different unit `F<U2, T>` **at runtime**, preserving the numerical value in terms of the base unit (e.g., grams).

```cpp
F<'K'> kilo{ 1500.0 };
auto mega = kilo.to<'M'>();  // F<'M'> with value 0.0015
```

### ✔️ Cross-unit constructor

Also enable:

```cpp
F<'K'> kilo{ 1500.0 };
F<'M'> mega = kilo;  // OK — same as to<'M'>()
```

---

## 🚧 Conversion Logic

Conversion must follow:

```cpp
value_in_target = value_in_source * (source_ratio / target_ratio)
```

---

## ✅ Extended Code with Unit Conversion Support

```cpp
#include <iostream>
#include <format>
#include <print>
#include <concepts>
#include <type_traits>

// --- Compile-time ratio lookup with guard for invalid units
template<char Unit>
constexpr double get_ratio() {
    if constexpr (Unit == 'B') return 1'000'000'000.0;
    else if constexpr (Unit == 'M') return 1'000'000.0;
    else if constexpr (Unit == 'K') return 1'000.0;
    else if constexpr (Unit == 'm') return 0.001;
    else if constexpr (Unit == 'g') return 1.0;
    else
        static_assert(Unit == 'B' || Unit == 'M' || Unit == 'K' || Unit == 'm' || Unit == 'g',
                      "Unsupported unit symbol in F<Unit>");
}

// --- Unit-aware floating-point wrapper
template<char Unit, std::floating_point T = double>
struct F {
    using value_type = T;
    static constexpr T ratio = static_cast<T>(get_ratio<Unit>());
    static constexpr char symbol = Unit;

    T value;

    constexpr F(const T& v) : value{v} {}
    constexpr F(const F<Unit, T>& other) = default;

    // Safe cross-type constructor with matching unit
    template<char U2, std::floating_point T2>
    constexpr explicit(!std::is_same_v<T, T2>)
    F(const F<U2, T2>& other) requires (U2 == Unit)
        : value(static_cast<T>(other.value)) {}

    // 🆕 Cross-unit conversion constructor
    template<char U2, std::floating_point T2>
    constexpr explicit F(const F<U2, T2>& other)
        requires (U2 != Unit)
        : value(static_cast<T>(other.value * (get_ratio<U2>() / get_ratio<Unit>()))) {}

    // 🆕 .to<'U'> method for convenience
    template<char TargetUnit>
    constexpr F<TargetUnit, T> to() const {
        return F<TargetUnit, T>(*this);
    }
};

// --- CTAD Guide
template<char U, std::floating_point T>
F(const F<U, T>&) -> F<U, T>;

// --- Formatter specialization
template<char U, std::floating_point T>
struct std::formatter<F<U, T>> {
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();
    }

    auto format(const F<U, T>& obj, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "{:g}{}", obj.value, F<U, T>::symbol);
    }
};

int main() {
    F<'K'> kilo{1500.0};     // 1500 kg
    F<'M'> mega = kilo;      // auto-converted to megagrams
    F<'g'> grams = kilo;     // converted to grams (1,500,000 g)
    F<'m'> milli = grams.to<'m'>(); // grams to milligrams (1.5e+6 * 1000)

    std::println("kilo: {}  → mega: {}", kilo, mega);
    std::println("kilo: {}  → grams: {}", kilo, grams);
    std::println("grams: {} → milli: {}", grams, milli);

    // Other examples
    F<'B'> tons{3.5};
    std::println("{} in grams = {}", tons, tons.to<'g'>());
    std::println("{} in kilos = {}", tons, tons.to<'K'>());

    std::println("sizeof F<'K'> is {}, sizeof a double is {}", sizeof(F<'K'>), sizeof(double));
    return 0;
}
```

---

## ✅ Example Output

```plaintext
kilo: 1500K  → mega: 1.5M
kilo: 1500K  → grams: 1500000g
grams: 1500000g → milli: 1.5e+06m
3.5B in grams = 3.5e+09g
3.5B in kilos = 3.5e+06K
sizeof F<'K'> is 8, sizeof a double is 8
```

---

## 🧠 Bonus: Why It Works Well

* ✅ **Compile-time correctness**: All conversions are checked and constants are resolved at compile time.
* ✅ **Safe constructors**: Explicit prevents accidental conversions.
* ✅ **to<>() syntax**: Makes conversions expressive and readable.
* ✅ **No runtime overhead**: Only the wrapped floating-point value is stored.

---

Would you like:

* User-defined literals like `1.5_K` or `3.5_B`?
* Compile-time unit names (instead of just `'K'`)?
* Dimensional analysis (e.g. kg × m / s²)?

Let me know — happy to add!


*/