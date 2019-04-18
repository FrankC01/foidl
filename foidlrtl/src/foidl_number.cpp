/*
    foidl_numbers.c
    Library main entry

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#define NUMBER_IMPL
#include <foidlrt.h>
#include <stdio.h>
#include <m_apm_lc.h>

static PFRTAny fend = (PFRTAny) &_end.fclass;
static PFRTAny fnil = (PFRTAny) &_nil.fclass;
static PFRTAny ftrue = (PFRTAny) &_true.fclass;
static PFRTAny ffalse = (PFRTAny) &_false.fclass;

/*
    Handle library re-entrancy issues
*/
#ifdef _MSC_VER
static HANDLE   number_lock;
#else
static pthread_mutex_t number_lock;
#endif

static void init_locks() {
#ifdef _MSC_VER
    number_lock = CreateMutex(
        NULL,              // default security attributes
        FALSE,             // initially not owned
        NULL);
#else
    pthread_mutex_init(&number_lock, NULL);
#endif
}

static int get_nlock() {
#ifdef _MSC_VER
    return (int) WaitForSingleObject(
            number_lock,    // handle to mutex
            INFINITE);
#else
    return pthread_mutex_lock(&number_lock);
#endif
}

static int release_nlock() {
#ifdef _MSC_VER
    return (int) ReleaseMutex(number_lock);
#else
    return pthread_mutex_unlock(&number_lock);
#endif
}

// Register a new number

EXTERNC PFRTAny  foidl_reg_number(char *s) {
    get_nlock();
    M_APM   numapm = m_apm_init();
    if(strlen(s) > 2 && s[0] == '0') {
        switch(s[1]) {
            case 'x':
            case 'X':
                m_apm_set_long(numapm,
                    strtol(&s[2], NULL, 16));
                break;
            case 'b':
            case 'B':
                m_apm_set_long(numapm,
                    strtol(&s[2], NULL, 2));
                break;
            default:
                m_apm_set_string(numapm, s);
        }
    }
    else {
        m_apm_set_string(numapm, s);
    }
    release_nlock();
    return allocAny(scalar_class, number_type, (void *)numapm);
}

EXTERNC PFRTAny foidl_reg_intnum(long long v) {
    get_nlock();
    M_APM   numapm = m_apm_init();
    m_apm_set_long(numapm,v);
    release_nlock();
    return allocAny(scalar_class, number_type, (void *)numapm);
}

// Conversions and helpers

static ft num_buffersize(M_APM numapm) {
    ft cnt = (m_apm_sign(numapm) == -1) ? 1 : 0;
    cnt += m_apm_significant_digits(numapm);
    cnt += (m_apm_is_integer(numapm) == 0) ? 1 : 0;
    return cnt+1;

}

EXTERNC ft  number_tostring_buffersize(PFRTAny num) {
    return num_buffersize((M_APM) num->value);
}

EXTERNC char *number_tostring(PFRTAny num) {
    M_APM   numapm = (M_APM) num->value;
    char *foo = (char *) foidl_xall(num_buffersize(numapm));
    if(m_apm_is_integer(numapm) == 1) {
        m_apm_to_integer_string(foo, numapm);
    }
    else {
        m_apm_to_fixpt_string(foo, -1, numapm);
    }
    return foo;
}

static M_APM _make_abs(M_APM numapm) {
    M_APM   numabs = m_apm_init();
    m_apm_absolute_value(numabs, numapm);
    return numabs;
}

EXTERNC long long number_tolong(PFRTAny num) {
    get_nlock();
    M_APM   numapm = (M_APM) num->value;
    M_APM   numabs = _make_abs(numapm);
    char *nts = (char *)foidl_xall(num_buffersize(numabs));
    m_apm_to_fixpt_string(nts, -1, numabs);
    long long res = strtoll(nts, NULL, 10);
    foidl_xdel(nts);
    m_apm_free(numabs);
    release_nlock();
    return res;
}

static int _number_toint(PFRTAny num) {
    return (int) number_tolong(num);
}

EXTERNC ft number_toft(PFRTAny num) {
    return (ft) number_tolong(num);
}

// Equality

static int _mnum_equality(PFRTAny flhs, PFRTAny frhs) {
    return m_apm_compare((M_APM) flhs->value, (M_APM) frhs->value);
}

EXTERNC PFRTAny foidl_num_lt(PFRTAny flhs, PFRTAny frhs) {
    if(_mnum_equality(flhs, frhs) == -1)
        return ftrue;
    else
        return ffalse;
}

EXTERNC PFRTAny foidl_num_lteq(PFRTAny flhs, PFRTAny frhs) {
    if(_mnum_equality(flhs, frhs) <= 0)
        return ftrue;
    else
        return ffalse;
}

EXTERNC PFRTAny foidl_num_equal(PFRTAny flhs, PFRTAny frhs) {
    if(_mnum_equality(flhs, frhs) == 0)
        return ftrue;
    else
        return ffalse;
}

EXTERNC PFRTAny foidl_num_nequal(PFRTAny flhs, PFRTAny frhs) {
    if(_mnum_equality(flhs, frhs) == 0)
        return ffalse;
    else
        return ftrue;
}

EXTERNC PFRTAny foidl_num_gt(PFRTAny flhs, PFRTAny frhs) {
    if(_mnum_equality(flhs, frhs) == 1)
        return ftrue;
    else
        return ffalse;
}

EXTERNC PFRTAny foidl_num_gteq(PFRTAny flhs, PFRTAny frhs) {
    if(_mnum_equality(flhs, frhs) >= 0)
        return ftrue;
    else
        return ffalse;
}

// Odd/Even

EXTERNC PFRTAny foidl_num_odd(PFRTAny flhs) {
    if(m_apm_is_odd((M_APM) flhs->value) == 1)
        return ftrue;
    else
        return ffalse;
}

EXTERNC PFRTAny foidl_num_even(PFRTAny flhs) {
    if(m_apm_is_even((M_APM) flhs->value) == 1)
        return ftrue;
    else
        return ffalse;
}

// Positive negative

EXTERNC PFRTAny is_number_positive(PFRTAny arg) {
    return (m_apm_sign((M_APM) arg->value) == -1) ? ffalse: ftrue;
}

EXTERNC PFRTAny is_number_negative(PFRTAny arg) {
    return (m_apm_sign((M_APM)arg->value) == -1) ? ftrue : ffalse;
}

EXTERNC PFRTAny is_number_integer(PFRTAny arg) {
    return (m_apm_is_integer((M_APM)arg->value) == 1) ? ftrue : ffalse;
}

// +, -, *, /

EXTERNC PFRTAny     foidl_num_add(PFRTAny flhs, PFRTAny frhs) {
    M_APM res = m_apm_init();
    m_apm_add(res, (M_APM)flhs->value,(M_APM)frhs->value);
    return allocAny(scalar_class, number_type, (void *)res);
}

EXTERNC PFRTAny     foidl_num_sub(PFRTAny flhs, PFRTAny frhs) {
    M_APM res = m_apm_init();
    m_apm_subtract(res, (M_APM)flhs->value, (M_APM)frhs->value);
    return allocAny(scalar_class, number_type, (void *)res);
}

EXTERNC PFRTAny     foidl_num_mul(PFRTAny flhs, PFRTAny frhs) {
    M_APM res = m_apm_init();
    m_apm_multiply(res, (M_APM)flhs->value,(M_APM)frhs->value);
    return allocAny(scalar_class, number_type, (void *)res);
}

EXTERNC PFRTAny     foidl_num_div(PFRTAny flhs, PFRTAny frhs) {
    M_APM res = m_apm_init();
    m_apm_divide(res, 10, (M_APM)flhs->value,(M_APM)frhs->value);
    return allocAny(scalar_class, number_type, (void *)res);
}

EXTERNC PFRTAny foidl_num_mod(PFRTAny arg1, PFRTAny arg2) {
    M_APM q = m_apm_init();
    M_APM r = m_apm_init();
    m_apm_integer_div_rem(q, r, (M_APM)arg1->value, (M_APM)arg2->value);
    m_apm_free(q);
    return allocAny(scalar_class, number_type, (void *)r);
}

// Functions on numbers

EXTERNC PFRTAny foidl_num_abs(PFRTAny arg) {
    if(arg->ftype != number_type)
        unknown_handler();
    M_APM numabs = (_make_abs((M_APM) arg->value));
    return allocAny(scalar_class, number_type, (void *)numabs);
}

EXTERNC PFRTAny foidl_num_neg(PFRTAny arg) {
    if(arg->ftype != number_type)
        unknown_handler();
    M_APM numneg = m_apm_init();
    m_apm_negate(numneg, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numneg);
}

EXTERNC PFRTAny foidl_num_factorial(PFRTAny arg) {
    if(arg->ftype != number_type)
        unknown_handler();
    M_APM numflr = m_apm_init();
    m_apm_factorial(numflr, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numflr);
}

EXTERNC PFRTAny foidl_num_floor(PFRTAny arg) {
    if(arg->ftype != number_type)
        unknown_handler();
    M_APM numflr = m_apm_init();
    m_apm_floor(numflr, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numflr);
}

EXTERNC PFRTAny foidl_num_ceil(PFRTAny arg) {
    if(arg->ftype != number_type)
        unknown_handler();
    M_APM numcil = m_apm_init();
    m_apm_ceil(numcil, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numcil);
}

EXTERNC PFRTAny foidl_num_round(PFRTAny decpl, PFRTAny arg) {
    if(arg->ftype != number_type || decpl->ftype != number_type)
        unknown_handler();
    M_APM numcil = m_apm_init();
    int decs = _number_toint(decpl);
    m_apm_round(numcil, decs, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numcil);
}

EXTERNC PFRTAny foidl_num_sqrt(PFRTAny decpl, PFRTAny arg) {
    if(arg->ftype != number_type || decpl->ftype != number_type)
        unknown_handler();
    M_APM numcil = m_apm_init();
    int decs = _number_toint(decpl);
    m_apm_sqrt(numcil, decs, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numcil);
}

EXTERNC PFRTAny foidl_num_sin(PFRTAny decpl, PFRTAny arg) {
    if(arg->ftype != number_type || decpl->ftype != number_type)
        unknown_handler();
    M_APM numcil = m_apm_init();
    int decs = _number_toint(decpl);
    m_apm_sin(numcil, decs, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numcil);
}

EXTERNC PFRTAny foidl_num_cos(PFRTAny decpl, PFRTAny arg) {
    if(arg->ftype != number_type || decpl->ftype != number_type)
        unknown_handler();
    M_APM numcil = m_apm_init();
    int decs = _number_toint(decpl);
    m_apm_cos(numcil, decs, (M_APM) arg->value);
    return allocAny(scalar_class, number_type, (void *)numcil);
}
// Shorthand macro

#define genint(lsym,v)  lsym = foidl_reg_intnum((long long) v)

// Global 0 - 16 setup

EXTERNC void foidl_rtl_init_numbers() {
    M_init_trig_globals();
    init_locks();
    //  Utilitity counters
    zero = allocAny(scalar_class, number_type, (void *)MM_Zero);
    one = allocAny(scalar_class, number_type, (void *)MM_One);
    two = allocAny(scalar_class, number_type, (void *)MM_Two);
    three = allocAny(scalar_class, number_type, (void *)MM_Three);
    four = allocAny(scalar_class, number_type, (void *)MM_Four);
    five = allocAny(scalar_class, number_type, (void *)MM_Five);
    /*
    genint(zero,0x00);
    genint(one,0x01);
    genint(two,0x02);
    genint(three,0x03);
    genint(four,0x04);
    genint(five,0x05);
    */
    genint(six,0x06);
    genint(seven,0x07);
    genint(eight,0x08);
    genint(nine,0x09);
    // genint(ten,0x0A);
    ten = allocAny(scalar_class, number_type, (void *)MM_Ten);
    genint(eleven,0x0B);
    genint(twelve,0x0C);
    genint(thirteen,0x0D);
    genint(fourteen,0x0E);
    genint(fifteen,0x0F);
    genint(sixteen,0x10);

    return;
}
