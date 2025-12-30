# Lich's Portfolio: Game Design Document

## Executive Summary

**Title:** Lich's Portfolio  
**Genre:** Idle/Incremental Strategy with Dark Fantasy Satire  
**Platform:** PC (Linux, Windows, macOS via Steam)  
**Target Audience:** Fans of idle games, financial simulators, and dark humor  
**Unique Selling Point:** Play economics on a *centuries* timescale as an immortal undead investor

**Elevator Pitch:**  
You are Malachar the Undying, a lich who discovered that the true path to power isn't conquering kingdoms—it's *owning* them through compound interest. Slumber for decades, wake to fortunes or ruin, and build an economic empire that outlasts civilizations.

---

## Narrative Framework

### The Protagonist: Malachar the Undying

**Origin:**  
In life, Malachar was a court wizard and financial advisor to the Kingdom of Valdris. He witnessed three generations of kings squander fortunes on wars, only to die and leave their children to repeat the cycle. The pattern disgusted him.

When he achieved lichdom 847 years ago, he made a vow: he would never again serve mortal whims. Instead, he would *own* the systems that mortals serve.

**Philosophy:**  
Malachar views the mortal economy as a complex organism—beautiful in its patterns, predictable in its follies. He doesn't hate mortals; he finds them... *quaint*. Like watching ants build elaborate colonies that he knows will flood next spring.

His dark humor emerges from genuine detachment. When a kingdom falls, he notes it the way a gardener notes a frost: unfortunate, but the perennials will return.

**Voice Examples:**
- *"Another holy crusade against my kind. Bullish on weapons manufacturing, bearish on church donations lasting beyond the initial fervor."*
- *"The Third Valorian Empire has collapsed. The Fourth will arise in approximately 80 years, once the survivors' grandchildren forget why they were fighting. I should acquire land in the interim."*
- *"My agent reports that King Aldric is 'beloved by his people.' Translation: he's spending beyond his means. Short the crown's bonds."*

### The World: Aethermere

A high-fantasy world with a 2,000+ year recorded history, featuring:

**Geography:**
- **The Sundered Continent:** Primary landmass, divided into ever-shifting kingdoms
- **The Merchant Isles:** Neutral trading hub, relatively stable (the lich's primary banking base)
- **The Theocratic North:** Religious states, hostile to undead, but their crusades create predictable economic cycles
- **The Eastern Wastes:** Other liches, necromancer kingdoms, volatile but occasionally profitable

**Economic Realities:**
- Magic exists but is costly—this creates interesting arbitrage opportunities
- Gods are real and occasionally intervene (market disruption events)
- Other immortals exist (dragons, elder vampires, rival liches) and compete for long-term resources
- The average kingdom lifespan is 150-300 years; empires occasionally last 500+

### Tone & Themes

**Primary Tone:** Dark comedy meets financial thriller

**Themes:**
1. **The Long View:** What would you do differently if you could wait 500 years for returns?
2. **Detachment vs. Engagement:** Malachar *could* conquer the world, but that's so much *work*
3. **Systemic Thinking:** Individual mortals are unpredictable; systems are not
4. **The Price of Immortality:** Malachar has outlived everything he once cared about—except numbers going up

**NOT the tone:**
- Not grimdark or edgy
- Not a power fantasy about domination
- Not cruel for cruelty's sake—Malachar is *indifferent*, not sadistic

---

## Core Gameplay Loop

```
┌─────────────────────────────────────────────────────────────┐
│                    THE ETERNAL CYCLE                        │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│    ┌──────────┐                          ┌──────────┐      │
│    │  WAKE    │                          │  SLUMBER │      │
│    │          │                          │          │      │
│    │ • Review │                          │ • Time   │      │
│    │   results│                          │   passes │      │
│    │ • Handle │                          │ • Events │      │
│    │   crises │                          │   occur  │      │
│    │ • Collect│                          │ • Markets│      │
│    │   returns│                          │   shift  │      │
│    └────┬─────┘                          └────▲─────┘      │
│         │                                     │            │
│         ▼                                     │            │
│    ┌──────────┐                          ┌────┴─────┐      │
│    │  ANALYZE │                          │  COMMIT  │      │
│    │          ├─────────────────────────►│          │      │
│    │ • World  │                          │ • Set    │      │
│    │   state  │      ┌──────────┐        │   slumber│      │
│    │ • Trends │      │  DECIDE  │        │   length │      │
│    │ • Risks  │─────►│          │───────►│ • Lock   │      │
│    │ • Agents │      │ • Invest │        │   orders │      │
│    │          │      │ • Agents │        │ • Set    │      │
│    └──────────┘      │ • Hedge  │        │   wakers │      │
│                      │ • Divest │        └──────────┘      │
│                      └──────────┘                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

### Phase 1: WAKE

When you wake from slumber:

1. **Time Report:** How long you slept, what year it is, major events timeline
2. **Portfolio Summary:** Net worth change, individual investment performance
3. **Crisis Queue:** Events requiring immediate attention (if any)
4. **Agent Status:** Who's alive, who died, who was compromised

**Emotional Beat:** The anticipation of "what happened while I was gone?"

### Phase 2: ANALYZE

Review the current state of the world:

1. **World Map:** Political boundaries (changed during slumber), economic zones
2. **Market Data:** Commodity prices, trade route values, currency strength
3. **Intelligence Reports:** Agent observations, rumors, predictions
4. **Competitor Activity:** What other immortals have been doing

**Emotional Beat:** Pattern recognition, spotting opportunities others miss

### Phase 3: DECIDE

Make your moves:

1. **Investment Actions:**
   - Buy/sell assets
   - Open/close positions
   - Diversify or concentrate

2. **Agent Actions:**
   - Deploy new agents
   - Promote agent families
   - Eliminate compromised agents
   - Plant succession plans

3. **Contingency Actions:**
   - Set "wake me if" conditions
   - Hedge against specific disasters
   - Place dormant orders ("if X happens, do Y")

**Emotional Beat:** Strategic planning, the satisfaction of a complex plan

### Phase 4: COMMIT

Lock in your decisions for the slumber:

1. **Choose Slumber Duration:** 10 years to 500+ years
2. **Set Wake Conditions:** Automatic wake triggers
3. **Final Review:** Last chance to adjust

**Emotional Beat:** The leap of faith, committing to your predictions

### Phase 5: SLUMBER (Idle Phase)

Time passes. This happens whether the game is open or closed.

- **If Game Open:** Accelerated time with narrative events scrolling by
- **If Game Closed:** Time passes at real-world ratio (configurable)

**Emotional Beat:** The idle satisfaction, anticipation building

---

## Investment Systems

### Asset Classes

#### 1. Real Property (Low Risk, Low Return, Very Stable)

**Types:**
- Agricultural land
- Urban real estate
- Mines and quarries
- Forests (lumber rights)

**Mechanics:**
- Generates steady income per decade
- Survives political upheaval (unless burned/salted)
- Appreciates slowly
- Can be improved with investment

**Risks:**
- Natural disasters
- War damage
- Peasant revolts (temporary income loss)
- Magical blights

#### 2. Trade & Commerce (Medium Risk, Medium Return)

**Types:**
- Trade route shares
- Merchant guild stakes
- Shipping concerns
- Market monopolies

**Mechanics:**
- Higher returns than land
- Affected by political stability
- Can create synergies (own the route AND the goods)
- Requires active management (agents)

**Risks:**
- Route disruption (war, monsters, competition)
- Guild politics
- Piracy
- New route discovery (makes old routes obsolete)

#### 3. Financial Instruments (Variable Risk/Return)

**Types:**
- Kingdom bonds (loan to crown)
- Merchant notes (loan to traders)
- Insurance pools
- Currency holdings

**Mechanics:**
- Bonds: Fixed return if kingdom survives, total loss if it falls
- Notes: Shorter term, higher rates, default risk
- Insurance: Collect premiums, pay out on disasters
- Currency: Arbitrage between kingdoms, inflation hedge

**Risks:**
- Default
- Currency devaluation
- Inflation
- Political seizure

#### 4. Magical Assets (High Risk, High Return)

**Types:**
- Artifact speculation
- Spell component futures
- Magical creature farms
- Enchantment workshops

**Mechanics:**
- Artifacts appreciate unpredictably (adventurer discoveries)
- Components follow supply/demand (wars increase demand for healing)
- Creature farms are labor-intensive but lucrative
- Workshops require skilled (living) workers

**Risks:**
- Magical accidents
- Regulatory changes (magic bans)
- Adventurer interference
- Competitor sabotage

#### 5. Political Influence (Special Asset Class)

**Types:**
- Noble house backing
- Church donations (buying tolerance)
- Crown advisory positions (through proxies)
- Spy networks

**Mechanics:**
- Doesn't generate direct income
- Multiplies other investments' effectiveness
- Provides early warning of political shifts
- Can prevent hostile actions against you

**Risks:**
- Exposure
- Regime change
- Internal betrayal
- Competing influence

#### 6. Dark Investments (Hidden Asset Class, Unlocked Later)

**Types:**
- Undead labor contracts
- Soul trading
- Curse insurance (selling protection from your own curses)
- Necromantic services

**Mechanics:**
- Extremely profitable
- Must be kept secret from mortal institutions
- Requires specific infrastructure
- Can destabilize regions if overused

**Risks:**
- Discovery (crusades, asset seizure)
- Competition from other undead
- Divine intervention
- Moral hazard (destabilizing your own markets)

---

## Agent System

### The Mortal Problem

As a lich, you cannot openly participate in mortal society. You need agents—mortals who act on your behalf. But mortals have an unfortunate tendency to *die*.

### Agent Types

#### 1. Individual Agents

**Recruitment:**
- Find mortals with useful skills and flexible morals
- Offer wealth, power, forbidden knowledge, or extended life
- Some know they serve a lich; others think they serve a "mysterious benefactor"

**Capabilities:**
- Execute specific transactions
- Gather intelligence
- Maintain cover businesses
- Eliminate problems

**Limitations:**
- Lifespan: 40-80 years typically
- Loyalty degrades over time
- Can be killed, captured, or turned
- Knowledge dies with them

#### 2. Agent Families

**Mechanics:**
- Cultivate a bloodline of loyal servants
- Each generation is trained from birth
- Family secret: they serve an immortal master
- Create traditions and codes to maintain loyalty

**Benefits:**
- Continuity across generations
- Institutional knowledge
- Deeper infiltration possible
- Self-recruiting

**Risks:**
- Family feuds
- Ambitious members may betray
- Bloodline can be exterminated
- Takes decades to establish

#### 3. Cults

**Mechanics:**
- Religious organization worshipping you (or pretending to worship a false god)
- Members are fanatically loyal
- Self-perpetuating through conversion

**Benefits:**
- Large numbers
- High loyalty
- Useful for large operations
- Can operate semi-openly

**Risks:**
- Attracts attention
- Heresy investigations
- Internal schisms
- Difficult to control

#### 4. Bound Servants (Unlocked Later)

**Types:**
- Lesser undead (mindless, limited utility)
- Intelligent undead (expensive, useful)
- Bound souls (the most capable but ethically... questionable)

**Benefits:**
- Immortal (solves the dying problem)
- Absolutely loyal (magical binding)
- Can operate in places mortals cannot

**Risks:**
- Obviously unnatural
- Attracts crusaders
- Expensive to maintain
- May draw other liches' attention

### Agent Management UI

```
┌─────────────────────────────────────────────────────────────┐
│ AGENT: MATTHIAS CRANE (Merchant Prince Cover)              │
├─────────────────────────────────────────────────────────────┤
│ Age: 47 / Est. Remaining: 15-25 years                      │
│ Loyalty: 87% (declining)                                    │
│ Competence: High (Trade, Finance, Negotiation)             │
│ Cover Status: Secure                                        │
│ Knowledge Level: Knows true master                          │
├─────────────────────────────────────────────────────────────┤
│ CURRENT ASSIGNMENTS:                                        │
│ • Managing Valdris trade route investments                  │
│ • Cultivating successor (son, age 22)                      │
│ • Intelligence gathering on Crown finances                  │
├─────────────────────────────────────────────────────────────┤
│ SUCCESSION PLAN:                                            │
│ Primary: Marcus Crane (son) - Training complete            │
│ Secondary: Helena Crane (daughter) - Not yet initiated     │
├─────────────────────────────────────────────────────────────┤
│ ACTIONS: [Reassign] [Reward] [Eliminate] [Succession Now]  │
└─────────────────────────────────────────────────────────────┘
```

---

## Time & Slumber Mechanics

### Slumber Duration Options

| Duration | Benefits | Risks | Unlock |
|----------|----------|-------|--------|
| 10 years | Safe, see immediate results | Minimal compound growth | Start |
| 25 years | One generation passes | Agent turnover begins | Start |
| 50 years | Significant growth potential | Political shifts likely | Start |
| 100 years | Major compound returns | Kingdoms may fall | Level 3 |
| 250 years | Generational wealth | Empires reshape | Level 7 |
| 500+ years | Legendary returns | Everything can change | Level 15 |

### Time Passage Mechanics

**During Slumber, the simulation runs:**

1. **Yearly Tick:** Basic economic calculations
2. **Decade Tick:** Major events, agent aging, political shifts
3. **Generation Tick (25 years):** Agent succession, cultural shifts
4. **Era Tick (100 years):** Civilizational changes, technology shifts

### Event System

**Event Categories:**

1. **Economic Events:**
   - Market booms/busts
   - Trade route discovery/collapse
   - Currency crises
   - Resource discoveries

2. **Political Events:**
   - Succession crises
   - Wars (civil and foreign)
   - Revolutions
   - Diplomatic shifts

3. **Magical Events:**
   - Artifact discoveries
   - Magical disasters
   - Divine interventions
   - Planar incursions

4. **Personal Events:**
   - Agent deaths/betrayals
   - Discovery attempts
   - Competitor actions
   - Crusades against undead

### Wake Conditions

You can set conditions that automatically wake you:

**Financial Triggers:**
- Net worth drops below X
- Specific investment fails
- Market crash detected
- Opportunity threshold reached

**Security Triggers:**
- Agent network compromised
- Discovery imminent
- Crusade approaches your territory
- Competitor directly attacks

**Opportunity Triggers:**
- Kingdom collapse (acquisition opportunity)
- Artifact appears on market
- Specific event occurs
- Investment matures

---

## Progression Systems

### Primary Progression: Phylactery Upgrades

Your phylactery isn't just your soul container—it's your operational center.

**Upgrade Trees:**

#### Temporal Mastery
- Increase maximum slumber duration
- Improve time-skip efficiency
- Unlock era-scale investments
- Gain "temporal arbitrage" ability (act on future knowledge)

#### Network Expansion
- Increase agent capacity
- Unlock agent family mechanics
- Unlock cult mechanics
- Unlock bound servant mechanics

#### Divination
- Better pre-slumber predictions
- Early warning of disasters
- Competitor activity visibility
- Market trend analysis

#### Resilience
- Survive longer without income
- Resist discovery
- Weather economic disasters
- Recover from setbacks faster

#### Dark Arts (Hidden Tree)
- Unlock dark investment category
- Necromantic infrastructure
- Soul trading
- Undead labor markets

### Secondary Progression: Influence

**Influence** represents your invisible hand in mortal affairs.

**Influence Categories:**
- **Economic:** Ability to manipulate markets
- **Political:** Ability to influence governments
- **Magical:** Ability to affect magical markets
- **Underground:** Ability to operate in shadows

**Influence Effects:**
- Higher influence = better rates, more information, more options
- Influence is regional (must build in each area)
- Influence can be lost through events

### Prestige System (Meta Progression)

**When do you prestige?**
When your phylactery is destroyed (either intentionally or through catastrophic failure).

**What carries over:**
- **Echoes:** Memories of past lives that provide passive bonuses
- **Artifacts:** Unique items discovered that persist
- **Codex Entries:** Unlocked lore and strategies
- **Meta Currencies:** Spent on permanent upgrades

**Prestige Narrative:**  
Liches can create backup phylacteries. When destroyed, you awaken in a new body, centuries later, with fragmentary memories. The world has changed. Your empire is dust. But you remember... *patterns*.

---

## World Generation & Simulation

### Initial World State

The game generates a world with:

**Fixed Elements:**
- Major geographic features
- Ancient history (pre-lich)
- Magical laws and limitations
- Other immortal competitors

**Variable Elements:**
- Current political map
- Starting kingdom characteristics
- Active conflicts
- Economic state

### Kingdom Simulation

Each kingdom has:

**Attributes:**
- Stability (0-100): Likelihood of collapse
- Prosperity (0-100): Economic health
- Military (0-100): War capability
- Culture (0-100): Resistance to change
- Tolerance (0-100): Acceptance of magic/undead

**Behaviors:**
- Kingdoms with low stability may collapse
- Prosperous kingdoms expand; poor ones contract
- High military kingdoms start wars
- Low tolerance kingdoms launch crusades

### Economic Simulation

**Macro Systems:**
- Trade flows between regions
- Currency value fluctuations
- Boom/bust cycles (predictable with enough data)
- Technology advancement (slow but impactful)

**Micro Systems:**
- Individual investment performance
- Agent actions and outcomes
- Competitor behavior
- Random events

---

## User Interface Design

### Main Screen (Awakened State)

```
┌─────────────────────────────────────────────────────────────────────┐
│ LICH'S PORTFOLIO                              Year: 1,247 A.C.     │
│ Net Worth: 2,847,392 gp    Change: +847% (last century)            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  ┌─────────────────────────────────────────────────────────────┐   │
│  │                     [WORLD MAP]                             │   │
│  │                                                             │   │
│  │   Political boundaries, economic zones, your holdings       │   │
│  │   marked. Hotspots highlighted.                             │   │
│  │                                                             │   │
│  └─────────────────────────────────────────────────────────────┘   │
│                                                                     │
│  ┌──────────────────┐  ┌──────────────────┐  ┌─────────────────┐   │
│  │ PORTFOLIO        │  │ AGENTS           │  │ INTELLIGENCE    │   │
│  ├──────────────────┤  ├──────────────────┤  ├─────────────────┤   │
│  │ Real Estate  23% │  │ Active: 12       │  │ 3 Urgent        │   │
│  │ Trade        31% │  │ Families: 3      │  │ 7 Important     │   │
│  │ Financial   18% │  │ Cults: 1         │  │ 12 Routine      │   │
│  │ Magical     15% │  │                  │  │                 │   │
│  │ Political   8%  │  │ Succession due:  │  │ [View All]      │   │
│  │ Dark         5% │  │ 2 this decade    │  │                 │   │
│  └──────────────────┘  └──────────────────┘  └─────────────────┘   │
│                                                                     │
│  [INVEST]  [AGENTS]  [CONTINGENCIES]  [DIVINATION]  [SLUMBER]      │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Slumber Configuration Screen

```
┌─────────────────────────────────────────────────────────────────────┐
│ PREPARE FOR SLUMBER                                                 │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  DURATION: [◀] 100 years [▶]                                       │
│                                                                     │
│  ┌───────────────────────────────────────────────────────────┐     │
│  │ PROJECTED OUTCOMES (based on current trends)              │     │
│  ├───────────────────────────────────────────────────────────┤     │
│  │                                                           │     │
│  │ Portfolio Growth:   +340% to +890% (most likely: +520%)  │     │
│  │ Kingdom Stability:  Valdris 67%→23%, Merchant Isles 89%  │     │
│  │ Agent Survival:     7 of 12 likely to have successors    │     │
│  │ Risk Events:        ~3 major, ~12 moderate               │     │
│  │                                                           │     │
│  │ DIVINATION WARNS:                                         │     │
│  │ • Theocratic crusade likely in 40-60 years               │     │
│  │ • Trade route disruption possible (war in east)          │     │
│  │ • Currency crisis in Valdris within 30 years             │     │
│  │                                                           │     │
│  └───────────────────────────────────────────────────────────┘     │
│                                                                     │
│  WAKE CONDITIONS:                                                   │
│  [✓] Net worth drops below 1,000,000 gp                            │
│  [✓] Crusade approaches within 100 leagues                         │
│  [✓] Agent network drops below 5                                   │
│  [ ] Major acquisition opportunity                                  │
│  [ ] Competitor direct action                                       │
│                                                                     │
│  DORMANT ORDERS:                                                    │
│  • If Valdris falls: Acquire crown bonds at 10% face value         │
│  • If crusade: Liquidate visible magical assets                    │
│  • [Add Order...]                                                   │
│                                                                     │
│  [REVIEW PORTFOLIO]  [REVIEW AGENTS]  [ENTER SLUMBER]              │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Wake Report Screen

```
┌─────────────────────────────────────────────────────────────────────┐
│ AWAKENING REPORT                                                    │
│ You slumbered for 97 years, 4 months, 12 days                      │
│ Current Year: 1,344 A.C.                                           │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│  WAKE REASON: Scheduled awakening                                   │
│                                                                     │
│  ══════════════════════════════════════════════════════════════    │
│                                                                     │
│  PORTFOLIO SUMMARY                                                  │
│  ─────────────────                                                  │
│  Starting Worth:    2,847,392 gp                                   │
│  Current Worth:    14,892,847 gp                                   │
│  Change:           +423%                                            │
│                                                                     │
│  Best Performer:   Valdris Reconstruction Bonds (+2,400%)          │
│  Worst Performer:  Eastern Trade Routes (-89%, war damage)         │
│                                                                     │
│  ══════════════════════════════════════════════════════════════    │
│                                                                     │
│  MAJOR EVENTS (Chronological)                                       │
│  ─────────────────────────────                                      │
│  Year 1,251: Valdris Civil War begins                              │
│  Year 1,258: Valdris Crown defaults on bonds                       │
│  Year 1,259: Your agent acquires bonds at 8% (per dormant order)   │
│  Year 1,267: War ends, new dynasty honors debts                    │
│  Year 1,289: Theocratic Crusade (your holdings unaffected)         │
│  Year 1,312: Eastern Empire collapses, trade disrupted             │
│  Year 1,334: Agent Family Crane: succession to 5th generation      │
│                                                                     │
│  [View Full Timeline]                                               │
│                                                                     │
│  ══════════════════════════════════════════════════════════════    │
│                                                                     │
│  URGENT MATTERS (1)                                                 │
│  ─────────────────                                                  │
│  ⚠ Agent Helena Crane requests audience (succession dispute)       │
│                                                                     │
│  [CONTINUE TO MAIN SCREEN]                                          │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Narrative Events & Writing Samples

### Event: Kingdom Collapse

```
┌─────────────────────────────────────────────────────────────────────┐
│ EVENT: THE FALL OF VALDRIS (Year 1,258)                            │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ The Kingdom of Valdris, which you have watched rise and fall       │
│ three times now, is falling again.                                 │
│                                                                     │
│ The proximate cause is King Aldric IV's decision to mint           │
│ copper coins with a thin gold wash and call them "golden crowns."  │
│ The merchants noticed. Then the peasants noticed. Then the         │
│ army—unpaid in anything of value for six months—noticed.           │
│                                                                     │
│ Your holdings in Valdris:                                          │
│ • Crown Bonds: 12,000 gp face value (current value: uncertain)     │
│ • Valdris Trade Guild: 8% stake                                    │
│ • Agricultural land: 2,400 acres                                   │
│                                                                     │
│ Your agents report:                                                │
│                                                                     │
│ MATTHIAS CRANE: "The bonds are worthless paper now. But the        │
│ rebels need financing. Whoever wins will need to honor SOME        │
│ debts to maintain credit. We could acquire defaulted bonds at      │
│ copper prices and wait."                                           │
│                                                                     │
│ SISTER VEX (Cult): "The chaos provides opportunity. Several        │
│ noble houses face extinction. Their lands and secrets could        │
│ be acquired... permanently."                                       │
│                                                                     │
│ [Acquire Bonds] [Acquire Land] [Extract Assets] [Observe]          │
│                                                                     │
│                    MALACHAR'S OBSERVATION                           │
│ "Ah, Valdris. I remember when your great-great-grandmother         │
│ defaulted on bonds I held. Her great-great-grandson seems no       │
│ wiser. Perhaps I should factor hereditary financial                │
│ incompetence into my models."                                       │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Event: Agent Succession

```
┌─────────────────────────────────────────────────────────────────────┐
│ EVENT: THE CRANE SUCCESSION                                         │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Matthias Crane is dying.                                           │
│                                                                     │
│ At 73, he has served you for 41 years—longer than most. His        │
│ hands, once steady enough to forge documents undetectable by       │
│ the Crown's inspectors, now tremble too much to hold a quill.      │
│ But his mind remains sharp.                                        │
│                                                                     │
│ He has summoned his children—Marcus (47) and Helena (44)—to        │
│ inform them of the family's true purpose.                          │
│                                                                     │
│ Marcus has been groomed for this. He knows, intellectually,        │
│ what he serves. But knowing and KNOWING are different things.      │
│ When Matthias speaks your name, Marcus goes pale.                  │
│                                                                     │
│ Helena is a surprise. She was meant to marry into the nobility,    │
│ to extend the family's cover. But her husband died, and she        │
│ returned. She shows no fear when she learns the truth.             │
│                                                                     │
│ "I suspected," she says. "No mortal builds what Father built       │
│ in one lifetime. I assumed a partnership with... something."       │
│                                                                     │
│ Matthias asks for your decision: who inherits his mantle?          │
│                                                                     │
│ MARCUS: Trained, competent, but fear may compromise him.           │
│ HELENA: Untrained, but adaptable. Unknown loyalty ceiling.         │
│ BOTH: Split responsibilities. Potential for rivalry.               │
│                                                                     │
│ [Choose Marcus] [Choose Helena] [Choose Both] [Delay Decision]     │
│                                                                     │
│                    MALACHAR'S OBSERVATION                           │
│ "Four generations of Cranes have served me now. Matthias was       │
│ the best of them—the right balance of competence, ambition,        │
│ and fear. I wonder if lightning strikes twice in the same          │
│ bloodline. Usually it doesn't. Usually I'm disappointed."          │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

### Event: Competitor Action

```
┌─────────────────────────────────────────────────────────────────────┐
│ EVENT: THE PALE COUNTESS MOVES                                      │
├─────────────────────────────────────────────────────────────────────┤
│                                                                     │
│ Your divinations detected it three decades ago: another immortal   │
│ was consolidating shipping in the Merchant Isles. You assumed      │
│ it was a dragon—they love hoarding trade rights.                   │
│                                                                     │
│ You were wrong.                                                     │
│                                                                     │
│ The Pale Countess—an elder vampire whose existence you had         │
│ considered a regional legend—has revealed herself to your          │
│ agent network. Not through violence. Through a letter.             │
│                                                                     │
│ "To the Ancient of Valdris," it reads. "I know you exist. You      │
│ know I exist. Our interests overlap but need not conflict.         │
│ I propose a division of spheres: you take the continent's          │
│ interior, I take the coasts and isles. We avoid each other's       │
│ agents. We share intelligence on mortal crusades. We do not        │
│ compete for the same assets. Consider this an offer between        │
│ equals who need not become enemies. Reply via the usual methods.   │
│ — Countess Serathine"                                               │
│                                                                     │
│ Your Analysis:                                                      │
│ • She knows more about you than you knew about her                 │
│ • Her proposal limits your coastal expansion but offers security   │
│ • Vampires are territorial; this is unusual diplomacy              │
│ • She may be genuine, or probing for weakness                      │
│                                                                     │
│ [Accept Terms] [Counter-Propose] [Decline] [Feign Ignorance]       │
│                                                                     │
│                    MALACHAR'S OBSERVATION                           │
│ "Another immortal. How tedious. I had hoped my only competition    │
│ would be compound interest itself. Still, she writes well.         │
│ Clear terms, no threats, mutual benefit framing. Either she's      │
│ genuinely civilized, or she's very good at pretending.             │
│ Both are useful traits in a potential... colleague."               │
│                                                                     │
└─────────────────────────────────────────────────────────────────────┘
```

---

## Technical Implementation Notes

### Simulation Architecture

**Core Simulation Loop:**

```
simulation_tick(years):
    for each year in years:
        # Economic simulation
        update_trade_flows()
        update_commodity_prices()
        check_economic_events()
        
        # Political simulation
        for each kingdom:
            update_stability()
            check_succession()
            check_wars()
            check_collapse()
        
        # Agent simulation
        for each agent:
            age_agent()
            check_mortality()
            check_loyalty()
            check_discovery()
        
        # Player portfolio
        for each investment:
            calculate_returns()
            apply_events()
        
        # Event generation
        generate_decade_events() if year % 10 == 0
        generate_generation_events() if year % 25 == 0
        
        # Wake condition check
        if check_wake_conditions():
            wake_player()
            return
```

### Save System

**Save Data Structure:**

```yaml
save_version: 1
world_seed: 847293
current_year: 1344
total_years_played: 497

player:
  net_worth: 14892847
  phylactery_level: 7
  unlocks: [dark_investments, century_slumber, bound_servants]
  
world_state:
  kingdoms:
    - id: valdris
      stability: 67
      prosperity: 45
      ruling_dynasty: "Aldric"
      # ... etc
  
  economic_state:
    trade_routes: [...]
    commodity_prices: {...}
    
portfolio:
  investments:
    - type: crown_bond
      kingdom: valdris
      face_value: 12000
      purchase_price: 960
      purchase_year: 1259
    # ... etc

agents:
  individuals: [...]
  families: [...]
  cults: [...]

event_history:
  - year: 1251
    type: civil_war
    kingdom: valdris
    # ... etc

meta_progression:
  echoes: 3
  artifacts: [orb_of_foresight]
  codex_unlocks: [valdris_history, vampire_lore]
```

### Idle Mechanics

**Offline Progression:**

When game is closed:
1. Record current timestamp
2. On reopen: calculate elapsed real time
3. Convert to game time (configurable ratio)
4. Run simulation for that duration
5. Present results on wake

**Configurable Time Ratios:**
- Casual: 1 real hour = 1 game year
- Standard: 1 real hour = 10 game years
- Hardcore: 1 real hour = 50 game years

---

## Monetization & Steam Strategy

### Launch Pricing

**Target Price:** $14.99 USD

**Rationale:**
- Complex enough to justify mid-tier pricing
- Idle game audience comfortable with $10-20 range
- Room for launch discount (10-15%)
- Sustainable for long-term updates

### Post-Launch Content

**Free Updates:**
- Balance adjustments
- Quality of life improvements
- Minor content additions
- Bug fixes

**Paid DLC Potential:**
- New world regions with unique mechanics
- New immortal protagonists (play as vampire, dragon, etc.)
- Deep-dive mechanics (detailed kingdom management, detailed agent stories)
- Soundtrack release

### Steam Features

**Achievements:**
- "First Million" - Reach 1,000,000 gp net worth
- "Centennial" - Complete a 100-year slumber
- "Dynasty" - Maintain an agent family for 5 generations
- "Hostile Takeover" - Acquire a kingdom's entire debt
- "Patient Investor" - Hold a single investment for 500 years
- "Undead Hand of the Market" - Trigger an economic crisis
- Secret achievements for dark path content

**Trading Cards:** Yes (6 cards, lich/world/agent themes)

**Cloud Saves:** Yes (essential for idle game)

**Workshop Support:** Future consideration for mod support

---

## Development Milestones

### Phase 1: Core Loop (MVP)

**Goal:** Playable idle loop with basic investments

**Features:**
- Single world region
- Basic investment types (land, trade, bonds)
- Simple agent system (individuals only)
- Time passage and wake mechanics
- Basic event system
- Minimal prestige (phylactery upgrades)

**Duration:** 6-8 weeks

### Phase 2: Depth

**Goal:** Full investment and agent systems

**Features:**
- All investment types
- Agent families and cults
- Political influence system
- Competitor AI (one other immortal)
- Rich event generation
- Full prestige system

**Duration:** 8-10 weeks

### Phase 3: Polish

**Goal:** Release quality

**Features:**
- Full narrative content
- UI/UX polish
- Sound and music
- Tutorial system
- Balance pass
- Save system hardening

**Duration:** 4-6 weeks

### Phase 4: Release Prep

**Goal:** Steam release

**Features:**
- Achievements
- Trading cards
- Store page assets
- Press kit
- Beta testing
- Launch marketing

**Duration:** 2-4 weeks

---

## Appendix: Glossary of Terms

| Term | Definition |
|------|------------|
| **Slumber** | The idle period where time passes and investments grow |
| **Wake** | The active period where players make decisions |
| **Phylactery** | The lich's soul container; also the upgrade system |
| **Agent** | Mortal servants who act on the player's behalf |
| **Dormant Order** | Pre-programmed actions that execute during slumber |
| **Wake Condition** | Triggers that end slumber early |
| **Influence** | Abstract resource representing hidden power in regions |
| **Divination** | The prediction/forecasting system |
| **Prestige** | Meta-progression through phylactery destruction |
| **Echo** | Memories carried through prestige |
| **Era** | 100-year period; major simulation unit |

---

## Appendix: Competitive Analysis

### Similar Games

**Universal Paperclips:**
- Similarity: Long-term thinking, exponential growth
- Difference: LP has narrative, economic complexity

**A Dark Room:**
- Similarity: Idle with narrative reveals
- Difference: LP is more simulation-focused

**Cookie Clicker / Idle Champions:**
- Similarity: Core idle mechanics
- Difference: LP has strategic depth, narrative focus

**Victoria / Crusader Kings (Paradox):**
- Similarity: Historical simulation, long timeframes
- Difference: LP is idle-focused, smaller scope, humor

### Unique Position

"The only idle game where you think in centuries, not seconds."

---

*This document is a living design reference. Update as development proceeds.*
