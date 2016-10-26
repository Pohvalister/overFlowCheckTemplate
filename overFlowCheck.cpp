#include <iostream>
#include <limits>

struct checkedOverFlowException {
    checkedOverFlowException(std::string const &str) {
        message = str;
    }

    std::string getMessage() {
        return message;
    }

    ~checkedOverFlowException() {}

private:
    std::string message;
};

struct signedBasedTag {
};
struct unsignedBasedTag {
};

template<typename T, bool B>//false
struct singedTraits {
    typedef unsignedBasedTag tag;
};
template<typename T>
struct singedTraits<T, true> {
    typedef signedBasedTag tag;
};

template<typename T>
struct valueTraitDef {
    typedef typename singedTraits<T, std::numeric_limits<T>::is_signed>::tag trait;
};

template<typename U, typename V>
struct is_same {
    static bool const value = false;
};
template<typename T>
struct is_same<T, T> {
    static bool const value = true;
};

template<bool condition, typename T>
struct enable_if;
template<typename T>
struct enable_if<true, T> {
    typedef T type;
};

template<typename T>
struct checked {
    checked() {
        val = 0;
    }

    checked(T number) {
        val = number;
    }
    /*checked<T> &operator=(checked<T> const &other) {
        val = other.val;
        return *this;
    }*/

    template<typename X>
    friend checked<X> operator+(checked<X> const &, checked<X> const &);

    template<typename X>
    friend checked<X> operator-(checked<X> const &, checked<X> const &);

    template<typename X>
    friend checked<X> operator*(checked<X> const &, checked<X> const &);

    template<typename X>
    friend checked<X> operator/(checked<X> const &, checked<X> const &);

    checked<T> operator-() const;

private:
    T val;
};

//check...OverFlow - true if OK
template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, signedBasedTag>::value, bool>::type
checkAddOverFlow(C a, C b) {//signed-add
    return !((b > 0) && (a > (std::numeric_limits<C>::max() - b)) ||
             (b < 0) && (a < (std::numeric_limits<C>::min() - b)));
};

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, unsignedBasedTag>::value, bool>::type
checkAddOverFlow(C a, C b) {//unsigned-add
    return !(std::numeric_limits<C>::max() - a < b);
};

template<typename X>
checked<X> operator+(checked<X> const &a, checked<X> const &b) {
    if (checkAddOverFlow(a.val, b.val))
        return checked<X>(a.val + b.val);
    throw checkedOverFlowException("add overflow\n");
}

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, signedBasedTag>::value, bool>::type
checkSubOverFlow(C a, C b) {//signed-sub
    return !((b < 0) && (a > (std::numeric_limits<C>::max() + b))
             || (b > 0) && (a < (std::numeric_limits<C>::min() + b)));
};

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, unsignedBasedTag>::value, bool>::type
checkSubOverFlow(C a, C b) {//unsigned-sub
    return a >= b;
};

template<typename X>
checked<X> operator-(checked<X> const &a, checked<X> const &b) {
    if (checkSubOverFlow(a.val, b.val))
        return checked<X>(a.val - b.val);
    throw checkedOverFlowException("sub overflow\n");
}

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, signedBasedTag>::value, bool>::type
checkMulOverFlow(C a, C b) {//signed-mul
    C min = std::numeric_limits<C>::min();
    C max = std::numeric_limits<C>::max();
    return (a == 0) || (b == 0) || (a == -1 && b != min) || (b == -1 && a != min)
           || (std::abs(a) < max / std::abs(b));
};

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, unsignedBasedTag>::value, bool>::type
checkMulOverFlow(C a, C b) {//unsigned-mul
    return (a == 0) || (b == 0) || (a < std::numeric_limits<C>::max() / b);
};

template<typename X>
checked<X> operator*(checked<X> const &a, checked<X> const &b) {
    if (checkMulOverFlow(a.val, b.val))
        return checked<X>(a.val * b.val);
    throw checkedOverFlowException("mul overflow\n");
}

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, signedBasedTag>::value, bool>::type
checkDivOverFlow(C a, C b) {//signed-div
    return !(b == -1 && a == std::numeric_limits<C>::min());
};

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, unsignedBasedTag>::value, bool>::type
checkDivOverFlow(C a, C b) {//unsigned-div
    return true;
};

template<typename X>
checked<X> operator/(checked<X> const &a, checked<X> const &b) {
    if (checkDivOverFlow(a.val, b.val))
        return checked<X>(a.val / b.val);
    throw checkedOverFlowException("div overflow\n");
}

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, signedBasedTag>::value, bool>::type
checkNegOverFlow(C a) {//signed-neg
    return (a != std::numeric_limits<C>::min());
};

template<typename C>
typename enable_if<is_same<typename valueTraitDef<C>::trait, unsignedBasedTag>::value, bool>::type
checkNegOverFlow(C a) {//unsigned-neg
    return (a == 0);
};

template<typename X>
checked<X> checked<X>::operator-() const {
    if (checkNegOverFlow(val))
        return checked<X>(val * X(-1));
    throw checkedOverFlowException("neg overflow\n");
}

int main() {
    typedef long long int type1;
    checked<type1> tmp1(100);
    checked<type1> tmp2(std::numeric_limits<type1>::min());
    try { tmp1 + tmp2; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { tmp1 - tmp2; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { tmp1 * tmp2; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { tmp1 / tmp2; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { -tmp2; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }

    typedef unsigned int type2;
    checked<type2> tmp3(std::numeric_limits<type2>::max());
    checked<type2> tmp4(10);
    try { tmp3 + tmp4; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { tmp3 - tmp4; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { tmp4 - tmp3; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { tmp3 * tmp4; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { tmp3 / tmp4; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }
    try { -tmp3; }
    catch (checkedOverFlowException &e) { std::cout << e.getMessage(); }

    return 0;
}