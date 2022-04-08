-----------------------------------
-- Area: Southern_San_dOria_[S]
-----------------------------------
require("scripts/globals/zone")
-----------------------------------

zones = zones or {}

zones[xi.zone.SOUTHERN_SAN_DORIA_S] =
{
    text =
    {
        ITEM_CANNOT_BE_OBTAINED = 6384,  -- You cannot obtain the <item>. Come back after sorting your inventory.
        ITEM_OBTAINED           = 6390,  -- Obtained: <item>.
        GIL_OBTAINED            = 6391,  -- Obtained <number> gil.
        KEYITEM_OBTAINED        = 6393,  -- Obtained key item: <keyitem>.
        CARRIED_OVER_POINTS     = 7001,  -- You have carried over <number> login point[/s].
        LOGIN_CAMPAIGN_UNDERWAY = 7002,  -- The [/January/February/March/April/May/June/July/August/September/October/November/December] <number> Login Campaign is currently underway!<space>
        LOGIN_NUMBER            = 7003,  -- In celebration of your most recent login (login no. <number>), we have provided you with <number> points! You currently have a total of <number> points.
        GATE_IS_LOCKED          = 7103,  -- The gate is locked.
        DONT_HURT_GELTPIX       = 7106,  -- Don't hurt poor Geltpix! Geltpix's just a merchant from Boodlix's Emporium in Jeuno. Kingdom vendors don't like gil, but Boodlix knows true value of new money.
        MOG_LOCKER_OFFSET       = 7373,  -- Your Mog Locker lease is valid until <timestamp>, kupo.
        REGIME_CANCELED         = 7630,  -- Current training regime canceled.
        HUNT_ACCEPTED           = 7648,  -- Hunt accepted!
        USE_SCYLDS              = 7649,  -- You use <number> [scyld/scylds]. Scyld balance: <number>.
        HUNT_RECORDED           = 7660,  -- You record your hunt.
        OBTAIN_SCYLDS           = 7662,  -- You obtain <number> [scyld/scylds]! Current balance: <number> [scyld/scylds].
        HUNT_CANCELED           = 7666,  -- Hunt canceled.
        WYATT_DIALOG            = 11092, -- Ahhh, sorry, sorry. The name's Wyatt, an' I be an armor merchant from Jeuno. Ended up 'ere in San d'Oria some way or another, though.
        HOMEPOINT_SET           = 11122, -- Home point set!
        ITEM_DELIVERY_DIALOG    = 11223, -- If'n ye have goods tae deliver, then Nembet be yer man!
        DISTANCE_YOURSELF       = 11261, -- I advise you distance yourself from Lady Ulla. I know not your intentions, but am inclined to believe they are crooked.
        ONE_OF_THESE_CITIES     = 11264, -- So this is one of these cities I've heard so much of, is it? Hmph. Seems to be nothing more than a mass of people crowded into a noisy, confined space.
        ACHTELLE_FROM_ADOULIN   = 11273, -- I go by the name Achtelle. I hail from Adoulin Isle. Word has reached there that the dragoons and their wyverns have long since disappeared from these eastern lands. Tell me, is it true?
        NOTHING_OUT_OF_ORDINARY = 11753, -- You find nothing out of the ordinary.
        TOO_BUSY                = 11757, -- I am far too busy to speak with you now. Perhaps I'll have a little time later.
        RINGING_OF_STEEL        = 11807, -- Hah hah hah! The ringing of steel upon steel! The dripping stench of the battlefield! What could be better than war? The nobles can have their balls, I'll do my dance on the front lines!
        BASEBORN_PEASANT        = 11808, -- How dare a baseborn peasant raise [his/her] voice to a noble knight!? Begone, before I strike you down myself!
        KINGDOMS_DEFEAT         = 11809, -- The Kingdom's defeat at Jugner still stings. To avenge the souls of those lost on that fateful day, we must join hands and take up arms as one.
        NOT_ONCE_IN_400_YEARS   = 11810, -- Not once in the four hundred years since the dawn of the Royal Knights has the Kingdom found itself in such peril. Let us take up our pikes and stand our ground to ensure another four centuries of prosperity!
        HOLY_DOCTRINES_PROHIBIT = 11812, -- While our holy doctrines specifically prohibit the taking of another life, you need not hesitate on the battlefield. The Church has branded the beastmen enemies of Altana. The purification must begin! It is the will of the Goddess!!!
        YEARS_OF_TRAINING       = 11813, -- After years of training in the Far East, I return only to find my nation burning at the hands of the infernal beastman hordes. The heathens shall pay dearly... My work has only just begun.
        FINE_WARRIOR            = 11814, -- You have the look of a fine warrior. It is a pity you are not one of my Crimson Wolves.
        EYES_OF_THE_GODDESS     = 11815, -- The eyes of the Goddess are ever upon us. We must remain steadfast against the evils from without, as well as those from within.
        ALLIED_SIGIL            = 12924, -- You have received the Allied Sigil!
        DOOR_IS_FIRMLY_LOCKED   = 13550, -- The door is firmly locked...
        RETRIEVE_DIALOG_ID      = 15587, -- You retrieve <item> from the porter moogle's care.
        COMMON_SENSE_SURVIVAL   = 15661, -- It appears that you have arrived at a new survival guide provided by the Servicemen's Mutual Aid Network. Common sense dictates that you should now be able to teleport here from similar tomes throughout the world.
    },
    mob =
    {
    },
    npc =
    {
    },
}

return zones[xi.zone.SOUTHERN_SAN_DORIA_S]
