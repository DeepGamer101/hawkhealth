*** Settings ***
Documentation     HawkHealth full-system integration suite (the working reference).
Suite Setup       Setup
Suite Teardown    Teardown
Test Teardown     Test Teardown
Resource          ${RENODEKEYWORDS}
*** Variables ***
${PLATFORM}       ${CURDIR}/../platforms/hawkhealth_f767.repl
${ELF}            ${CURDIR}/../firmware/hawkhealth.elf
${UART}           sysbus.usart3
${M_BOOT}         system starting
${M_HEALTH}       health monitor up
${M_TEMP}         temp
${R_HIGH}         38\\.5.*HIGH
${R_CRIT}         88\\.0.*CRITICAL
${R_CMDHIGH}      36\\.8.*HIGH
${M_CMD_TEMP}     cmd TEMP now
${M_CMD_STOP}     cmd STOP
${M_CMD_START}    cmd START
*** Keywords ***
Boot System
    Execute Command           mach create
    Execute Command           machine LoadPlatformDescription @${PLATFORM}
    Execute Command           sysbus LoadELF @${ELF}
    Create Terminal Tester    ${UART}
    Start Emulation
Send Bytes
    [Arguments]    @{codes}
    FOR    ${c}    IN    @{codes}
        Execute Command    sysbus.usart3 WriteChar ${c}
    END
*** Test Cases ***
Boots And Streams Telemetry
    Boot System
    Wait For Line On Uart    ${M_BOOT}      timeout=15
    Wait For Line On Uart    ${M_HEALTH}    timeout=15
    Wait For Line On Uart    ${M_TEMP}      timeout=15

Injected Faults Raise Correct Alerts
    [Documentation]    38.5 reading -> HIGH (call 300); 88.0 reading -> CRITICAL (call 500).
    Boot System
    Wait For Line On Uart    ${R_HIGH}    timeout=90     treatAsRegex=true
    Wait For Line On Uart    ${R_CRIT}    timeout=120    treatAsRegex=true

Command Changes Alert Behavior
    [Documentation]    Lower temp threshold via UART -> a normal 36.8 reading now alerts HIGH.
    Boot System
    Wait For Line On Uart    ${M_HEALTH}      timeout=15
    Send Bytes    84  69  77  80  32  51  48  46  48  10      # "TEMP 30.0\n"
    Wait For Line On Uart    ${M_CMD_TEMP}    timeout=15
    Wait For Line On Uart    ${R_CMDHIGH}     timeout=30    treatAsRegex=true
    Send Bytes    83  84  79  80  10                          # "STOP\n"
    Wait For Line On Uart    ${M_CMD_STOP}    timeout=15
    Send Bytes    83  84  65  82  84  10                      # "START\n"
    Wait For Line On Uart    ${M_CMD_START}   timeout=15
