# Scattered thoughts on design ideas

## Goals
- Encourage exploration
- Encourage creative building
- Vanilla knowledge (locations, loot pools, etc.) is useful without being fully accurate

## Caves
- Ground is much more solid, to encourage following existing caves over digging straight paths
- Caves form a nearly connected network
  - Digging in the correct direction from a dead end should quickly find the next cave segment
- Biome-specific surface access
  - Forest: living trees
  - Desert: pyramid
  - Jungle: valleys under mahogany vine burst islands
  - Corruption: chasms
  - Crimson: tunnels

## Biomes/micro biomes
- Support overlap whenever possible
- Soft borders with biome crossover blend rules

## Evil biomes
- Infection, spreading tendrils from a central point
- Corruption: jagged, chaotic, bacterial
- Crimson: smooth, organic

## Resources
- Prefer fewer, but larger ore/gem deposits
- Environmental clues to hint at nearby deposits
- Locate more valuable resources deeper

## Structures
- Blend/integrate with surrounding environment
- Detailed enough to be interesting to find
- Not so detailed the player:
  - Feels bad removing it
  - In competition with it
  - Like someone else was building in their world
- Often ruined/decayed, to feel like part of the environment
- Exhibit a building technique that the player may be able learn/adapt
- Avoid repetition

## Dungeon
- Avoid useless paths
  - Starting from the entrance, there should be exactly one path to each room
- Rooms not accessible from the current path may still be visible, to hint there is more to find
- Hidden hallways via cracked bricks

## Jungle temple
- Enemies are fairly easy to handle (due to terrain)
- Challenge comes from navigating narrow corridors, filled with traps
- Player can choose difficulty by, for example, deciding to not disarm any traps

## Loot
- Theme chest contents based on its biome
- Roughly follow vanilla probabilities
- For progress locked chests (dungeon, shadow, temple, biome), update secondary loot items
  to be relevant to the game stage the player will gain access to them
- Protect against bad RNG skipping items/producing low value chests
- Add thematic, but low value, items to non-chest storage furniture

## Traps
- Integrate with the surrounding environment
- Thematic
- Fair: the more deadly a trap, the easier it should be to observe before triggering
