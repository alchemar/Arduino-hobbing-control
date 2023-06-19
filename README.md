# Arduino-hobbing-control
Stepper motor control for hobbing gears
Arduino project to control two stepper motors for hobbing on a mill.

One stepper is connected to a stepper that runs the spindle.

Second stepper controls a rotary axis with a 72:1 gear ratio

i2c LCD screen and buttons is used to select the number of teeth on the wheel. That sets an electronic gear ratio between both the motors and slaves the spindle off the rotary axis stepper.

Selecting run from the LCD screen and buttons causes the rotary axis to ramp up to a set speed.

Currently only using ratios that use integer value beteen micro steps on rotary axis, and spindle.

