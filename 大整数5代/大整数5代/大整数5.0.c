#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define MAX 1000

typedef struct {
    int d[MAX];
    int len;
    int sign;
} BigInt;

void init(BigInt* a) {
    memset(a->d, 0, sizeof(a->d));
    a->len = 1;  
    a->sign = 1;
}

void fromString(BigInt* a, char* s) {
    init(a);
    int start = 0, n = strlen(s);

    if (s[0] == '-') {
        a->sign = -1;
        start = 1;
    }

    a->len = 0;  // 这里重置为0，因为我们要重新填充
    for (int i = n - 1; i >= start; i--) {
        a->d[a->len++] = s[i] - '0';
    }

    // 处理特殊情况：-0
    if (a->len == 1 && a->d[0] == 0) {
        a->sign = 1;
    }
}

void print(BigInt* a) {
    if (a->sign == -1) printf("-");
    for (int i = a->len - 1; i >= 0; i--) {
        printf("%d", a->d[i]);
    }
}

// 比较绝对值大小
int cmpAbs(BigInt* a, BigInt* b) {
    if (a->len != b->len) return a->len - b->len;
    for (int i = a->len - 1; i >= 0; i--) {
        if (a->d[i] != b->d[i]) return a->d[i] - b->d[i];
    }
    return 0;
}

BigInt addAbs(BigInt* a, BigInt* b) {
    BigInt c;
    // 特殊初始化，不从1开始
    memset(c.d, 0, sizeof(c.d));
    c.len = 0;  // 关键：从0开始计数
    c.sign = 1;

    int carry = 0;
    int maxLen = a->len > b->len ? a->len : b->len;

    for (int i = 0; i < maxLen; i++) {
        int sum = a->d[i] + b->d[i] + carry;
        c.d[c.len++] = sum % 10;
        carry = sum / 10;
    }

    // 处理最后的进位
    if (carry) {
        c.d[c.len++] = carry;
    }

    return c;
}

BigInt subAbs(BigInt* a, BigInt* b) {
    BigInt c;
    // 同样需要从0开始
    memset(c.d, 0, sizeof(c.d));
    c.len = 0;
    c.sign = 1;

    int borrow = 0;
    for (int i = 0; i < a->len; i++) {
        int diff = a->d[i] - borrow;
        if (i < b->len) diff -= b->d[i];

        if (diff < 0) {
            diff += 10;
            borrow = 1;
        }
        else {
            borrow = 0;
        }
        c.d[c.len++] = diff;
    }

    // 去掉前导0，但要确保至少有一位
    while (c.len > 1 && c.d[c.len - 1] == 0) {
        c.len--;
    }

    return c;
}

BigInt add(BigInt* a, BigInt* b) {
    BigInt c;

    if (a->sign == b->sign) {
        c = addAbs(a, b);
        c.sign = a->sign;
    }
    else {
        if (cmpAbs(a, b) >= 0) {
            c = subAbs(a, b);
            c.sign = a->sign;
        }
        else {
            c = subAbs(b, a);
            c.sign = b->sign;
        }
    }

    // 处理结果为0的情况
    if (c.len == 1 && c.d[0] == 0) c.sign = 1;
    return c;
}

BigInt subtract(BigInt* a, BigInt* b) {
    BigInt negB = *b;
    negB.sign = -negB.sign;
    return add(a, &negB);
}

BigInt multiply(BigInt* a, BigInt* b) {
    BigInt c;
    memset(c.d, 0, sizeof(c.d));
    c.len = 0;
    c.sign = a->sign * b->sign;

    // 处理乘以0的情况
    if ((a->len == 1 && a->d[0] == 0) || (b->len == 1 && b->d[0] == 0)) {
        c.len = 1;  // 0的长度为1
        return c;
    }

    for (int i = 0; i < a->len; i++) {
        int carry = 0;
        for (int j = 0; j < b->len; j++) {
            int sum = a->d[i] * b->d[j] + c.d[i + j] + carry;
            c.d[i + j] = sum % 10;
            carry = sum / 10;
        }
        if (carry) {
            c.d[i + b->len] += carry;
        }
    }

    c.len = a->len + b->len;
    while (c.len > 1 && c.d[c.len - 1] == 0) c.len--;

    return c;
}

BigInt divide(BigInt* a, BigInt* b) {
    BigInt result, remainder;
    memset(result.d, 0, sizeof(result.d));
    result.len = 0;
    result.sign = 1;

    memset(remainder.d, 0, sizeof(remainder.d));
    remainder.len = 0;
    remainder.sign = 1;

    // 处理除数为0的情况
    if (b->len == 1 && b->d[0] == 0) {
        printf("错误：除数不能为0！\n");
        result.len = 1;  // 返回0
        return result;
    }

    // 处理被除数为0的情况
    if (a->len == 1 && a->d[0] == 0) {
        result.len = 1;  // 返回0
        return result;
    }

    result.sign = a->sign * b->sign;

    // 使用长除法算法（只实现整除）
    BigInt absA = *a;
    absA.sign = 1;
    BigInt absB = *b;
    absB.sign = 1;

    // 如果被除数小于除数，结果为0
    if (cmpAbs(&absA, &absB) < 0) {
        result.len = 1;  // 返回0
        return result;
    }

    // 创建一个临时数组来存储结果
    int res[MAX] = { 0 };
    int resLen = 0;

    // 从最高位开始处理
    for (int i = a->len - 1; i >= 0; i--) {
        // 将当前位加到余数中
        BigInt temp;

        // 将余数左移一位
        for (int j = remainder.len; j > 0; j--) {
            remainder.d[j] = remainder.d[j - 1];
        }
        if (remainder.len > 0) remainder.len++;
        remainder.d[0] = a->d[i];

        // 试商
        int quotient = 0;
        while (cmpAbs(&remainder, &absB) >= 0) {
            remainder = subAbs(&remainder, &absB);
            quotient++;
        }

        res[resLen++] = quotient;
    }

    // 反转结果
    for (int i = 0; i < resLen; i++) {
        result.d[i] = res[resLen - 1 - i];
    }
    result.len = resLen;

    // 去掉前导0
    while (result.len > 1 && result.d[result.len - 1] == 0) result.len--;

    return result;
}

int main() {
    BigInt a, b, result;
    char s1[100], s2[100];
    int choice;

    printf("大整数四则运算程序\n");
    printf("版本号：1.0\n");
    printf("2025年12月\n");
    printf("甘肃政法大学网安学院\n");
    printf("25信息安全C语言程序设计小组\n");
    printf("C程序设计兴趣小组\n\n");

    printf("请按enter进入下一页");
    getchar();

    system("cls");

    printf("大整数的四则运算\n");

    while (1) {
        printf("\n请选择操作：\n");
        printf("1. 加法\n");
        printf("2. 减法\n");
        printf("3. 乘法\n");
        printf("4. 除法（整除）\n");
        printf("5. 退出\n");
        printf("请输入选择（1-5）：");
        scanf("%d", &choice);

        // 清理scanf留下的换行符
        while (getchar() != '\n');

        if (choice == 5) break;
        if (choice < 1 || choice > 5) {
            printf("无效选择！\n");
            continue;
        }

        printf("\n请输入第一个大整数：");
        scanf("%s", s1);
        printf("请输入第二个大整数：");
        scanf("%s", s2);

        // 清理scanf留下的换行符
        while (getchar() != '\n');

        fromString(&a, s1);
        fromString(&b, s2);

        printf("\n计算结果：\n");

        switch (choice) {
        case 1:
            printf("%s + %s = ", s1, s2);
            result = add(&a, &b);
            print(&result);
            break;
        case 2:
            printf("%s - %s = ", s1, s2);
            result = subtract(&a, &b);
            print(&result);
            break;
        case 3:
            printf("%s × %s = ", s1, s2);
            result = multiply(&a, &b);
            print(&result);
            break;
        case 4:
            printf("%s ÷ %s = ", s1, s2);
            result = divide(&a, &b);
            print(&result);
            break;
        }
        printf("\n");
    }

    printf("\n程序结束，再见！\n");

    // 等待用户按任意键退出
    printf("按任意键退出...");
    getchar();

    return 0;
}