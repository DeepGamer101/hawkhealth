*** Settings ***
Suite Setup       Setup
Suite Teardown    Teardown
Test Teardown     Test Teardown
Resource          ${RENODEKEYWORDS}
*** Variables ***
${PLATFORM}       ${CURDIR}/../platforms/hawkhealth_f767.repl
${ELF}            ${CURDIR}/../firmware/hawkhealth_test.elf
${UART}           sysbus.usart3
*** Test Cases ***
Integration Testbench Passes
    Execute Command           mach create
    Execute Command           machine LoadPlatformDescription @${PLATFORM}
    Execute Command           sysbus LoadELF @${ELF}
    Create Terminal Tester    ${UART}
    Start Emulation
    Wait For Line On Uart     T3 HIGH temp @ call300 ... PASS    timeout=20
    Wait For Line On Uart     T4 CRIT spo2 @ call500 ... PASS    timeout=20
    Wait For Line On Uart     HAWKHEALTH_TEST_PASS              timeout=20
