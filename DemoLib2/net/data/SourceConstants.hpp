#pragma once

#include <cstdint>

static constexpr unsigned int SPROP_NUMFLAGBITS_NETWORKED = 16;

static constexpr unsigned int PROPINFOBITS_NUMPROPS = 10;
static constexpr unsigned int PROPINFOBITS_TYPE = 5;
static constexpr unsigned int PROPINFOBITS_FLAGS = SPROP_NUMFLAGBITS_NETWORKED;
static constexpr unsigned int PROPINFOBITS_NUMELEMENTS = 10;
static constexpr unsigned int PROPINFOBITS_NUMBITS = 7;

static constexpr unsigned int MAX_OSPATH = 260;

static constexpr unsigned int NETMSG_TYPE_BITS = 6;

static constexpr unsigned int EVENT_INDEX_BITS = 8;
static constexpr unsigned int MAX_EVENT_BITS = 9;

static constexpr unsigned int MAX_DECAL_INDEX_BITS = 9;
static constexpr unsigned int MAX_EDICT_BITS = 11;
static constexpr unsigned int MAX_EDICTS = 1 << MAX_EDICT_BITS;

static constexpr unsigned int COORD_INTEGER_BITS = 14;
static constexpr unsigned int COORD_FRACTIONAL_BITS = 5;
static constexpr unsigned int COORD_DENOMINATOR = 1 << COORD_FRACTIONAL_BITS;
static constexpr float COORD_RESOLUTION = 1.0 / COORD_DENOMINATOR;

static constexpr unsigned int COORD_INTEGER_BITS_MP = 11;
static constexpr unsigned int COORD_FRACTIONAL_BITS_MP_LOWPRECISION = 3;
static constexpr unsigned int COORD_DENOMINATOR_LOWPRECISION = 1 << COORD_FRACTIONAL_BITS_MP_LOWPRECISION;
static constexpr float COORD_RESOLUTION_LOWPRECISION = 1.0 / COORD_DENOMINATOR_LOWPRECISION;

static constexpr unsigned int NORMAL_FRACTIONAL_BITS = 11;
static constexpr unsigned int NORMAL_DENOMINATOR = (1 << (NORMAL_FRACTIONAL_BITS)) - 1;
static constexpr float NORMAL_RESOLUTION = float(1.0 / NORMAL_DENOMINATOR);

static constexpr unsigned int MAX_SOUND_INDEX_BITS = 14;

static constexpr unsigned int MAX_USER_MSG_TYPE_BITS = 8;
static constexpr unsigned int MAX_USER_MSG_LENGTH_BITS = 11;
static constexpr unsigned int MAX_USER_MSG_LENGTH = 1 << MAX_USER_MSG_LENGTH_BITS;

static constexpr unsigned int MAX_ENTITY_MSG_LENGTH_BITS = 11;
static constexpr unsigned int MAX_ENTITY_MSG_LENGTH = 1 << MAX_ENTITY_MSG_LENGTH_BITS;

static constexpr unsigned int MAX_SERVER_CLASS_BITS = 9;
static constexpr unsigned int MAX_SERVER_CLASSES = 1 << MAX_SERVER_CLASS_BITS;

static constexpr unsigned int SP_MODEL_INDEX_BITS = 13;

static constexpr unsigned int MAX_STRINGTABLE_BITS = 5;
static constexpr unsigned int MAX_STRINGTABLES = 1 << MAX_STRINGTABLE_BITS;

static constexpr unsigned int NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS = 10;
static constexpr unsigned int NUM_NETWORKED_EHANDLE_BITS = MAX_EDICT_BITS + NUM_NETWORKED_EHANDLE_SERIAL_NUMBER_BITS;
static constexpr unsigned int INVALID_NETWORKED_EHANDLE_VALUE = (1 << NUM_NETWORKED_EHANDLE_BITS) - 1;

static constexpr unsigned int MAX_DATATABLES = 1024;
static constexpr unsigned int MAX_DATATABLE_PROPS = 4096;

static constexpr unsigned int DT_STRING_BITS = 9;
static constexpr unsigned int MAX_DT_STRING_LENGTH = (1 << DT_STRING_BITS);