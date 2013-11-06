/**
 * ------------------------------------------------------------------------------
 * Mariusz Wiśniewski
 * @email: poczta@mariuszbonifacy.pl
 * Czasowstrzymywacz version 1.0 - polish DIY Project based on Speedstack Timer
 * !!! VERSION WITHOUT SHIFTER *595 !!!
 * ------------------------------------------------------------------------------
 * AVR ATMEGA 328-PU Arduino / C Programming Language
 * @project website: www.mariuszbonifacy.pl
 * @website for speedcubers: www.kostkarubika.mariuszbonifacy.pl
 * What Czasowstrzymywacz really mean in English? Time-stop? Maybe, but this is too easy to pronounce.
 * ------------------------------------------------------------------------------
 * Big thanks to:
 * Jarosław Piersa - for advice in programming (fuzzy logic) - you are the men!
 * Check out his website: http://www-users.mat.umk.pl/~piersaj
 * My sister Kinga - for patience and alfa-beta-testing www.kocurkolandia.mariuszbonifacy.pl
 * Greetings to all speedcubers over the world !!! Blindfold rules !!!
 * ------------------------------------------------------------------------------
 */
//------------------------------ libraries -------------------------//
#include <StopWatch.h>
#include <CapacitiveSensor.h>
#include <stdlib.h>
#include <LiquidCrystal.h>
//------------------------ global variables -----------------------//
LiquidCrystal lcd(3, 4, 5, 6, 7, 8);

char scramble_array[60];
char last_scramble_array[60];

StopWatch sw_millis; //use stopwatch.h library

#define LED_BLUE 0 //declare leds
#define LED_GREEN 1
#define LED_RED 2

CapacitiveSensor right_sensor = CapacitiveSensor(13, 11);
CapacitiveSensor left_sensor = CapacitiveSensor(13, 12);

static FILE lcdout =
{ 0 }; // LCD character writer

void scramble(); //declare prototypes of functions
void time_measurements(uint32_t *pointer_all_time, uint8_t *pointer_number_of_times, uint32_t *pointer_miliseconds_to_average);
void main_menu();
void last_time_and_average(uint8_t *pointer_number_of_times, uint32_t *pointer_total_miliseconds, uint32_t results[]);
void led_red_function();
void led_green_function();
void led_blue_function();
void best_worst_average(uint8_t *pointer_number_of_times, uint32_t results[], uint32_t *pointer_best_average, uint32_t *pointer_worst_average);
static int lcd_putchar(char ch, FILE* stream);
void show_scramble();

//---------------------------------- setup ------------------------//
void setup()
{
	fdev_setup_stream(&lcdout, lcd_putchar, NULL, _FDEV_SETUP_WRITE);
	pinMode(LED_BLUE, OUTPUT); //activate pins in microcontroller
	pinMode(LED_GREEN, OUTPUT);
	pinMode(LED_RED, OUTPUT);

	randomSeed(analogRead(0)); //random number
	lcd.begin(20, 4); // turn on our lcd panel
	right_sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
	left_sensor.set_CS_AutocaL_Millis(0xFFFFFFFF);
	scramble(); //automatically generate the scramble when you turn on device
	pinMode(A5, INPUT_PULLUP); //switches as analog input
}
//--------------------- loop - heart of program -------------------//
void loop()
{
	//variables
	uint32_t results_array[150];
	static uint32_t total_miliseconds = 0, miliseconds_to_average = 0, all_time = 0;
	static uint8_t next_status = 1, plus_two_status = 1, dnf_status = 1, number_of_times = 0, buttons_status = 0, count = 4, right_value = 0,
			left_value = 0, old_right_value = 0, old_left_value = 0, check_out = 2;
	static uint32_t best_average = 0, worst_average = 0;

	//check out sensors status
	if (left_sensor.capacitiveSensor(50) > 30)
		left_value = HIGH;
	else
		left_value = LOW;

	if (right_sensor.capacitiveSensor(50) > 30)
		right_value = HIGH;
	else
		right_value = LOW;

	if ((analogRead(A5) > 170) && (analogRead(A5) < 240)) //read the status of button
	{
		dnf_status = 0;
		total_miliseconds = 0;
		miliseconds_to_average = 0;
		all_time = 0;
		number_of_times = 0;
		best_average = 0;
		worst_average = 0;
	}

	//if both were pressed
	if ((right_value == HIGH) && (left_value == HIGH)
			&& ((old_left_value == LOW) || (old_right_value == LOW)))
	{
		buttons_status = 1 - buttons_status;
		if (buttons_status == 1)
		{
			lcd.clear();
		}
		else
//-------------------------after time-stop-------------------------//
		{
			lcd.clear();

			if ((analogRead(A5) > 300) && (analogRead(A5) < 400)) //read the status of button
				next_status = 0;
			else
				next_status = 1;

			++number_of_times;
			results_array[number_of_times] = miliseconds_to_average; //save the time to array
			uint8_t k, dnf_or_plus = 0;
			for (k = 0; k < 60; k++)
				last_scramble_array[k] = scramble_array[k];
			total_miliseconds += results_array[number_of_times]; //save to average
			scramble(); //scramble next scramble!
			count = 4; //wait from 3 to 1
			check_out = 2;

			while (next_status == 1) //button reaction
			{
				if ((analogRead(A5) > 300) && (analogRead(A5) < 400)) //read the status of button
					next_status = 0;
				else
					next_status = 1;

				if ((analogRead(A5) > 450) && (analogRead(A5) < 550)) //read the status of button
					plus_two_status = 0;
				else
					plus_two_status = 1;

				if ((analogRead(A5) > 220) && (analogRead(A5) < 300)) //read the status of button
					dnf_status = 0;
				else
					dnf_status = 1;

				//show last time and avg and best and worst and.... what you really want
				last_time_and_average(&number_of_times, &total_miliseconds,
						results_array);

				//if we press +2 button
				if ((plus_two_status != 1) && (dnf_or_plus != 1))
				{
					total_miliseconds += 2000; // to average
					results_array[number_of_times] += 2000;
					++dnf_or_plus;
					lcd.setCursor(4, 3);
					lcd.print("Penalty: +2");
				}

				//if we press DNF button
				if ((dnf_status != 1) && (dnf_or_plus != 1))
				{
					total_miliseconds -= miliseconds_to_average;
					number_of_times--;
					++dnf_or_plus;
					lcd.setCursor(3, 3);
					lcd.print("DNF? Try again");
				}

				if ((analogRead(A5) > 170) && (analogRead(A5) < 220) && (dnf_or_plus != 1)) //read the status of button
				{
					dnf_status = 0;
					total_miliseconds = 0;
					miliseconds_to_average = 0;
					all_time = 0;
					number_of_times = 0;
					best_average = 0;
					worst_average = 0;
					lcd.setCursor(7, 3);
					lcd.print("RESET!");
					++dnf_or_plus;
				}

				//if we press NEXTbutton
				if (next_status != 1)
					break;
			}
		}
	}
	//buttons values are old now
	old_right_value = right_value;
	old_left_value = left_value;

	//--------------------- menu of lcd starts here -------------------//

//if buttons are not pressed
	if ((buttons_status == 0) && (right_value == LOW) && (left_value == LOW))
	{
		check_out = 2;
		sw_millis.reset();
		show_scramble();
	}
//if both buttons are pressed and waiting for hands release
	else if ((buttons_status == 1) && (right_value == HIGH)
			&& (left_value == HIGH))
	{
		while ((count > 1) && (check_out == 2))
		{
			led_red_function();
			count--;
			lcd.setCursor(5, 1);
			lcd.print("Get ready");
			lcd.setCursor(8, 2);
			lcd.print(count);
			lcd.print("...");
			delay(500);
			if (((left_sensor.capacitiveSensor(50) < 30)
					|| (right_sensor.capacitiveSensor(50) < 30))
					&& (count <= 4))
			{
				buttons_status = 0;
				count = 4;
				break;
			}
		}

		if (count == 1)
			check_out--;

		while ((count == 1) && (check_out == 1))
		{
			led_green_function();
			lcd.setCursor(2, 1);
			lcd.print("Get ready and...");
			lcd.setCursor(2, 2);
			lcd.print("RUN FOREST, RUN!");
			if ((left_sensor.capacitiveSensor(50) < 30)
					|| (right_sensor.capacitiveSensor(50) < 30))
			{
				count = 4;
				check_out = 0;
				break;
			}
		}
	}
// if we release right or right hand from button - start counting time
	else if ((buttons_status == 1)
			&& ((right_value == LOW) || (left_value == LOW))
			&& (check_out == 0))
	{
		time_measurements(&all_time, &number_of_times, &miliseconds_to_average);
	}
//if right or left is pressed while solving cube
	else if ((buttons_status == 1)
			&& ((right_value == HIGH) || (left_value == HIGH)))
	{
		time_measurements(&all_time, &number_of_times, &miliseconds_to_average);
	}
//if left is pressed in main menu
	else if ((buttons_status == 0) && (left_value == HIGH))
	{
		last_time_and_average(&number_of_times, &total_miliseconds,
				results_array);
		best_worst_average(&number_of_times, results_array, &best_average,
				&worst_average);
	}
//if right is pressed in main menu - last scramble
	else if ((buttons_status == 0) && (right_value == HIGH))
	{
		if (number_of_times == 0)
		{
			show_scramble();
		}
		else
		{
			led_red_function();
			lcd.setCursor(0, 0);
			lcd.print(' ');
			lcd.print(' ');
			lcd.print(' ');
			uint8_t k = 0;
			uint8_t space = 0, line_number = 0;
			uint8_t block = 0;
			while (1)
			{
				if (isAlphaNumeric(last_scramble_array[k])
						|| isWhitespace(last_scramble_array[k])
						|| isPunct(last_scramble_array[k]))
				{
					lcd.print(last_scramble_array[k]);
					block++;
				}

				if (last_scramble_array[k] == ' ')
				{
					while (block % 3 != 0)
					{
						lcd.print(' ');
						block++;
					}	// while
					space++;
					if (space % 5 == 0)
					{
						lcd.print(' ');
						lcd.print(' ');
						lcd.setCursor(0, ++line_number);
						lcd.print(' ');
						lcd.print(' ');
						lcd.print(' ');
						block = 0;
					}
					if (space == 20)
					{
						break;
					}
				}
				k++;
			}
			k = space = 0;
		}
	}
}
//----------------------------------- main_menu() -----------------//
void main_menu()
{
	show_scramble();
}
//--------------------------------- scramble ----------------------//
void scramble()
{
	uint8_t number_of_moves = 20, i = 0, j = 0;
	uint8_t number = 0, previous = 0, current_move = 0;

	while (current_move < number_of_moves)
	{
		//lets choose the number!
		number = random(6);
		j = i;
		if ((number == 0) && (previous != 0) && (previous != 1))
		{
			scramble_array[i] = 'U';
			i++;
			current_move++;
		}
		if ((number == 1) && (previous != 1) && (previous != 0))
		{
			scramble_array[i] = 'D';
			i++;
			current_move++;
		}
		if ((number == 2) && (previous != 2) && (previous != 3))
		{
			scramble_array[i] = 'R';
			i++;
			current_move++;
		}
		if ((number == 3) && (previous != 3) && (previous != 2))
		{
			scramble_array[i] = 'L';
			i++;
			current_move++;
		}
		if ((number == 4) && (previous != 4) && (previous != 5))
		{
			scramble_array[i] = 'F';
			i++;
			current_move++;
		}
		if ((number == 5) && (previous != 5) && (previous != 4))
		{
			scramble_array[i] = 'B';
			i++;
			current_move++;
		}

		previous = number;
		number = random(5);
		//add space or prefix or two
		if ((number > 2) && (i != j))
		{
			j = i;
			scramble_array[i] = ' ';
			i++;
		}
		if ((number <= 1) && (i != j))
		{
			j = i;
			scramble_array[i] = '\'';
			i++;
		}
		if ((number == 2) && (i != j))
		{
			j = i;
			scramble_array[i] = '2';
			i++;
		}
		//add spaces in array
		if ((number <= 1) && (i != j))
		{
			j = i;
			scramble_array[i] = ' ';
			i++;
		}
		if ((number == 2) && (i != j))
		{
			j = i;
			scramble_array[i] = ' ';
			i++;
		}
	}
}
//------------------------ show_scramble() ------------------------//
void show_scramble()
{
	led_red_function();
	lcd.setCursor(0, 0);
	lcd.print(' ');
	lcd.print(' ');
	lcd.print(' ');
	uint8_t k = 0;
	uint8_t space = 0, line_number = 0;
	uint8_t block = 0;
	while (1)
	{
		if (isAlphaNumeric(scramble_array[k]) || isWhitespace(scramble_array[k])
				|| isPunct(scramble_array[k]))
		{
			lcd.print(scramble_array[k]);
			block++;
		}

		if (scramble_array[k] == ' ')
		{
			while (block % 3 != 0)
			{
				lcd.print(' ');
				block++;
			}	// while
			space++;
			if (space % 5 == 0)
			{
				lcd.print(' ');
				lcd.print(' ');
				lcd.setCursor(0, ++line_number);
				lcd.print(' ');
				lcd.print(' ');
				lcd.print(' ');
				block = 0;
			}
			if (space == 20)
			{
				break;
			}
		}
		k++;
	}
	k = space = 0;
}
//---------------------- time_measurements() ----------------------//
void time_measurements(uint32_t *pointer_all_time,
		uint8_t *pointer_number_of_times,
		uint32_t *pointer_miliseconds_to_average)
{
	//clearing screen from rubbish, LOL. I know that it is easier to use lcd.clear(), but
	//this is slow and generate flickering. Moreover, I know this is hard way.
	//However spaces do not make a delay in time measurements!
	lcd.setCursor(0, 0);
	uint8_t q = 0;
	for (q = 0; q < 20; q++)
	{
		lcd.setCursor(q, 0);
		lcd.print(" ");

		lcd.setCursor(q, 3);
		lcd.print(" ");
	}
	lcd.setCursor(0, 1);
	lcd.print("    ");
	lcd.setCursor(0, 2);
	lcd.print("    ");
	lcd.setCursor(16, 1);
	lcd.print("    ");
	lcd.setCursor(16, 2);
	lcd.print("    ");

	led_blue_function();
	sw_millis.start();

	uint16_t hours = 0, minutes = 0, seconds = 0, miliseconds = 0;

	*pointer_all_time = sw_millis.elapsed() * 0.001;
	*pointer_miliseconds_to_average = sw_millis.elapsed();

	hours = ((*pointer_all_time / 3600));
	minutes = ((*pointer_all_time % 3600) / 60);
	seconds = ((*pointer_all_time % 3600) % 60);
	miliseconds = (sw_millis.elapsed() % 1000);

	lcd.setCursor(4, 1);
	lcd.print("Current time");
	lcd.setCursor(4, 2);
	fprintf(&lcdout, "%02u:%02u.%02u.%003u", hours, minutes, seconds,
			miliseconds);
}
//---------------------- last_time_and_average() ------------------//
void last_time_and_average(uint8_t *pointer_number_of_times,
		uint32_t *pointer_total_miliseconds, uint32_t results[])
{
	///////////////////////// zero: print some spaces!
	lcd.setCursor(5, 0);
	lcd.print("   ");
	lcd.setCursor(7, 1);
	lcd.print(" ");
	lcd.setCursor(9, 2);
	lcd.print("  ");

	///////////////////////// first of all: let's show our time

	uint16_t hours = 0, minutes = 0, seconds = 0, miliseconds = 0,
			last_time = 0;
	results[0] = 0;
	last_time = results[*pointer_number_of_times] * 0.001;
	hours = ((last_time / 3600));
	minutes = ((last_time % 3600) / 60);
	seconds = ((last_time % 3600) % 60);
	miliseconds = (results[*pointer_number_of_times] % 1000);

	lcd.setCursor(0, 0);
	lcd.print("Time:");
	lcd.setCursor(8, 0);
	fprintf(&lcdout, "%02u:%02u.%02u.%003u", hours, minutes, seconds,
			miliseconds);

	///////////////////////////////////  next: average

	lcd.setCursor(0, 1);
	fprintf(&lcdout, "%003d", *pointer_number_of_times); //------------------------------ ZMIANA

	lcd.setCursor(4, 1);

	uint16_t miliseconds_average = ((*pointer_total_miliseconds
			/ *pointer_number_of_times) % 1000);
	uint16_t seconds_average = (((*pointer_total_miliseconds / 1000)
			/ *pointer_number_of_times) % 60);
	uint16_t minutes_average = ((((*pointer_total_miliseconds / 1000) / 60)
			/ *pointer_number_of_times) % 60);
	uint16_t hours_average =
			(((*pointer_total_miliseconds / 1000) / 60 / 60)
					/ *pointer_number_of_times);

	if (*pointer_number_of_times == 0)
	{
		hours_average = 0;
		minutes_average = 0;
		seconds_average = 0;
		miliseconds_average = 0;
	}

	fprintf(&lcdout, "%02u:%02u.%02u.%003u", hours_average, minutes_average,
			seconds_average, miliseconds_average);

	///////////////////////////////////  next: best worst time

	static uint32_t best_time = 0, worst_time = 0;

	if (*pointer_number_of_times == 0)
	{
		best_time = 0;
		worst_time = 0;
	}

	if (*pointer_number_of_times >= 1)
	{
		best_time = results[1];
		worst_time = results[1];
		uint8_t k;
		for (k = 1; k <= *pointer_number_of_times; k++)
		{
			if (results[k] < best_time)
			{
				best_time = results[k];
			}
			else if (results[k] > worst_time)
			{
				worst_time = results[k];
			}
		}
	}
	//worst time - sometimes we suck
	uint16_t min_w, sec_w, mili_w, all_w;
	all_w = worst_time * 0.001;
	min_w = ((all_w % 3600) / 60);
	sec_w = ((all_w % 3600) % 60);
	mili_w = (worst_time % 1000);
	lcd.setCursor(11, 2);
	fprintf(&lcdout, "%02d:%02d.%003d", min_w, sec_w, mili_w);

	//best time - time to print it on lcd screen and shot the world!
	uint16_t min_b, sec_b, mili_b, all_b;
	all_b = best_time * 0.001;
	min_b = ((all_b % 3600) / 60);
	sec_b = ((all_b % 3600) % 60);
	mili_b = (best_time % 1000);
	lcd.setCursor(0, 2);
	fprintf(&lcdout, "%02d:%02d.%003d", min_b, sec_b, mili_b);
}
//----------------------- best_worst_average ----------------------//
void best_worst_average(uint8_t *pointer_number_of_times, uint32_t results[],
		uint32_t *pointer_best_average, uint32_t *pointer_worst_average)
{
	static uint8_t mini = 1, q = 0;
        static uint32_t all_averages[150];
	uint32_t sum = 0;
	mini = (*pointer_number_of_times - 9);
	if (*pointer_number_of_times >= 10)
	{

		for (q = mini; q <= *pointer_number_of_times; q++)
			sum += results[q];

		all_averages[mini] = (sum / 10);
		*pointer_best_average = all_averages[1];
		*pointer_worst_average = all_averages[1];
	}

	for (q = 1; q <= (*pointer_number_of_times - 9); q++)
	{
		if (all_averages[q] < *pointer_best_average)
		{
			*pointer_best_average = all_averages[q];
		}
		else if (all_averages[q] > *pointer_worst_average)
		{
			*pointer_worst_average = all_averages[q];
		}
	}

	//worst average - sometimes we suck
	uint16_t min_w, sec_w, mili_w, all_w;
	all_w = *pointer_worst_average * 0.001;
	min_w = ((all_w % 3600) / 60);
	sec_w = ((all_w % 3600) % 60);
	mili_w = (*pointer_worst_average % 1000);
	lcd.setCursor(11, 3);
	fprintf(&lcdout, "%02d:%02d.%003d", min_w, sec_w, mili_w);

	//best average - time to print it on lcd screen and shot the world!
	uint16_t min_b, sec_b, mili_b, all_b;
	all_b = *pointer_best_average * 0.001;
	min_b = ((all_b % 3600) / 60);
	sec_b = ((all_b % 3600) % 60);
	mili_b = (*pointer_best_average % 1000);
	lcd.setCursor(0, 3);
	fprintf(&lcdout, "%02d:%02d.%003d", min_b, sec_b, mili_b);

	//yes, again - spaces! again we must clear last message
	lcd.setCursor(9, 3);
	lcd.print("  ");
}
//---------------------------- led control ------------------------//
void led_red_function()
{
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_RED, HIGH);
	digitalWrite(LED_BLUE, LOW);
}
void led_green_function()
{
	digitalWrite(LED_GREEN, HIGH);
	digitalWrite(LED_RED, LOW);
	digitalWrite(LED_BLUE, LOW);
}
void led_blue_function()
{
	digitalWrite(LED_GREEN, LOW);
	digitalWrite(LED_RED, LOW);
	digitalWrite(LED_BLUE, HIGH);
}
//------------------------------ fprintf --------------------------//
static int lcd_putchar(char ch, FILE* stream)
{
	lcd.write(ch);
	return (0);
}
