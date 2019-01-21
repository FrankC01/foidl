/*
    foidl_numbers.c
    Library main entry

    Copyright Frank V. Castellucci
    All Rights Reserved
*/

#define NUMBER_IMPL
#include <foidlrt.h>
#include <stdio.h>
#include <m_apm.h>


PFRTAny  foidl_reg_number(char *s) {
    M_APM   numapm = m_apm_init();
    m_apm_set_string(numapm, s);
    return allocAny(scalar_class, number_type, (void *)numapm);
}

static ft num_buffersize(M_APM numapm) {
    ft cnt = (m_apm_sign(numapm) == -1) ? 1 : 0;
    cnt += m_apm_significant_digits(numapm);
    cnt += (m_apm_is_integer(numapm) == 0) ? 1 : 0;
    return cnt+1;

}

ft  number_tostring_buffersize(PFRTAny num) {
    return num_buffersize((M_APM) num->value);
}

char *number_tostring(PFRTAny num) {
    M_APM   numapm = (M_APM) num->value;
    char *foo = foidl_xall(num_buffersize(numapm));
    m_apm_to_fixpt_string(foo, -1, numapm);
    return foo;
}