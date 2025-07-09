west init -l u5
west update
west build -b stm32u5g9j_dk2 u5/app/
west flash