## Bugs

- [ ]  Drag timer does not reset when the cursor moves off the "draggable" element, allowing you to initialise the drag
  timer and then move the mouse elsewhere.

## To Do

- [x]  Use new UI framework for dialogue
- [ ]  Replace the literal model for items with the dota 2 style "boxes"
- [ ]  Cooldown timers on abilities
- [x] Improve placement of tooltips and abstract tooltips into a class.
- [ ] Make dialog box look like DOS2.
- [ ] Outline weapons, characters and clickable objects
- [ ] Killable, fetch and "interact with" quests

----

- [ ] Make terrain texture take into account the cursor's radius for spell casting
- [ ] Cylinders should be able to be placed on floor.
- [ ]  Add multiple rotated billboards as part of the particle system to see if that helps the blending from behind
  etc. (Look at thinmatrix video for hints, too).
- [ ]  Add decals with TextureTerrainOverlay on fireball hit
- [ ]  Lightning effect with rotated additive texture: https://www.youtube.com/watch?v=XVQDUcr6dwo
- [ ]  If camera cant see renderable, dont render it
- [ ]  If camera cant see animation, do not "UpdateAnimation" (Keep ticking the animation counter)
- [ ]  Enemy logic for moving towards player/starting combat is poor, right now.

----

- [x] Enable user to pickup and drop objects
- [x] Build UI framework.
- [x]  Camera "look at" should needs to look at the nearest hit floor object, rather than the world origin.
- [x]  There is a bug where if an enemy is dying and you hit them then they get back up
- [x]  Add a top/tailed cylinder and that fire effect with the gradient and you should get a nice shockwave type
  effect
- [x]  Rotate a mask of a texture and you should get the swipe you want
- [x]  CursorAbility should check if the move is "valid" before confirm is accepted (the ability indicator can decide
  what is valid or not?)
- [x]  Hold down mouse to move/attack.
- [x]  Make the slash torus follow the rotation of the player.
- [x] Animations need some form of queue/priority queue. (In system: a map of entity -> AnimationEnum).
- [x]  Bug where player cannot move after going near certain objects.