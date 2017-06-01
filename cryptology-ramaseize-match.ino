/* Defines, typedefs, constants */
typedef unsigned char GAME_STATE;

static const char * SEQUENCE_1 = "ABCDEFGH";
static const char * SEQUENCE_2 = "FACEHDBG";
static const char * SEQUENCE_3 = "DEAFGHCB";

static const char * SEQUENCES[3] = {
	SEQUENCE_1,
	SEQUENCE_2,
	SEQUENCE_3
};

/* Private Variables */

/* Private Functions */

static int find_state_in_sequence(GAME_STATE state, char * sequence)
{
	char state_letter = 'A' + state;
	int index = 0;
	for (index = 0; index < 8; index++)
	{
		if (sequence[index] == state_letter)
		{
			return index;
		}
	}
	return -1;
}

static bool game_state_can_advance(GAME_STATE state, int const * const switches_pressed, char const * const sequences[3])
{
	if (switches_pressed[0] == -1) { return false; }
	if (switches_pressed[1] == -1) { return false; }
	if (switches_pressed[2] == -1) { return false; }

	bool advance = true;

	int switch_states[3] = {
		sequences[0][switches_pressed[0]] - 'A',
		sequences[1][switches_pressed[1]] - 'A',
		sequences[2][switches_pressed[2]] - 'A'
	};

	advance &= switch_states[0] == state;
	advance &= switch_states[1] == state;
	advance &= switch_states[2] == state;

	return advance;
}

/* Public Functions */

void setup()
{
	Serial.begin(115200);
	
	int switches_pressed[] = {0, 0, 0};
	int i, j, k, s;
	for (s=0;s<8;s++)
	{
		for (i=0;i<8;i++)
		{
			for (j=0;j<8;j++)
			{
				for (k=0;k<8;k++)
				{
					switches_pressed[0] = i;
					switches_pressed[1] = j;
					switches_pressed[2] = k;

					if(game_state_can_advance(s, switches_pressed, SEQUENCES))
					{
						Serial.print(s); Serial.print(": ");
						Serial.print(i); Serial.print(',');
						Serial.print(j); Serial.print(',');
						Serial.println(k);
					}
				}
			}
		}
	}

	Serial.println("Done");
}

void loop()
{
}