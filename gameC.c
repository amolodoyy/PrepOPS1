#include <gpiod.h>
#include <stdio.h>
#include <unistd.h>

#define FIRST_LED -1
#define SECOND_LED -1
#define THIRD_LED -1

#define FIRST_BUTTON -1
#define SECOND_BUTTON -1
#define THIRD_BUTTON -1

#define MAX_BLINKS 10



int main(int argc, char **argv)
{
  const char *chipname = "gpiochip0";
  struct gpiod_chip *chip;
  struct gpiod_line *lineLED_1; 
  struct gpiod_line *lineLED_2;
  struct gpiod_line *lineLED_3; 

  struct gpiod_line *lineButton_1; 
  struct gpiod_line *lineButton_2; 
  struct gpiod_line *lineButton_3; 

  int i;
  int button_1_val, button_2_val, button_3_val;
  int sequence[MAX_BLINKS];

  // always start from the first diode
  sequence[0] = 1;

  for (int j = 1; j < MAX_BLINKS; j++)
    sequence[j] = 0;
  // Open GPIO chip
  chip = gpiod_chip_open_by_name(chipname);

  // Open GPIO lines
  lineLED_1 = gpiod_chip_get_line(chip, FIRST_LED);
  lineLED_2 = gpiod_chip_get_line(chip, SECOND_LED);
  lineLED_3 = gpiod_chip_get_line(chip, THIRD_LED);

  lineButton_1 = gpiod_chip_get_line(chip, FIRST_BUTTON);
  lineButton_2 = gpiod_chip_get_line(chip, SECOND_BUTTON);
  lineButton_3 = gpiod_chip_get_line(chip, THIRD_BUTTON);

  // Open LED lines for output
  gpiod_line_request_output(lineLED_1, "gameC", 0);
  gpiod_line_request_output(lineLED_2, "gameC", 0);
  gpiod_line_request_output(lineLED_3, "gameC", 0);

  // Open switch lines for input
  gpiod_line_request_input(lineButton_1, "gameC");
  gpiod_line_request_input(lineButton_2, "gameC");
  gpiod_line_request_input(lineButton_3, "gameC");

  // game logic
  i = 0;
  while (true) {

    // pick random number from 1 to 3
    for(int j = 0; j < MAX_BLINKS; j++){

        switch (sequence[j]){
            case 1:
             gpiod_line_set_value(lineLED_1, 1);
             break;
            case 2:
             gpiod_line_set_value(lineLED_2, 1);
             break;
            case 3:
             gpiod_line_set_value(lineLED_3, 1);
             break;
            default:
             return 1;
             break;
        }
        usleep(1000000);

        switch (sequence[j]){
            case 1:
             gpiod_line_set_value(lineLED_1, 0);
             break;
            case 2:
             gpiod_line_set_value(lineLED_2, 0);
             break;
            case 3:
             gpiod_line_set_value(lineLED_3, 0);
             break;
            default:
             return 1;
             break;
        }
    }




   /* gpiod_line_set_value(lineLED_1, (i & 1) != 0);
    gpiod_line_set_value(lineLED_2, (i & 2) != 0);
    gpiod_line_set_value(lineLED_3, (i & 4) != 0);

    // Read button status and exit if pressed
    val = gpiod_line_get_value(lineButton);
    if (val == 0) {
      break;
    }

    usleep(100000);
    i++;*/
  }

  // Release lines and chip
  gpiod_line_release(lineLED_1);
  gpiod_line_release(lineLED_2);
  gpiod_line_release(lineLED_3);

  gpiod_line_release(lineButton_1);
  gpiod_line_release(lineButton_2);
  gpiod_line_release(lineButton_3);

  gpiod_chip_close(chip);
  return 0;
}