## To Do

- [  ] Make terrain texture take into account the cursor's radius for spell casting
- [  ] Cylinders should be able to be placed on floor.
- [  ] Make game loop (waves going through portals)
- [  ] Animations need some form of queue/priority queue. (In system: a map of entity -> AnimationEnum).
- [  ]  Add mutliple rotated billboards as part of the particle system to see if that helps the blending from behind etc. (Look at thinmatrix video for hints, too).
- [  ]  Add decals with TextureTerrainOverlay on fireball hit
- [  ]  Lightning effect with rotated additive texture: https://www.youtube.com/watch?v=XVQDUcr6dwo
- [  ]  If camera cant see renderable, dont render it
- [  ]  If camera cant see animation, do not "UpdateAnimation" (Keep ticking the animation counter)
- [  ]  CursorAbility should check if the move is "valid" before confirm is accepted (the ability indicator can decide what is valid or not?)
- [  ]  Pass AttackData/AbilityData::VisualFX to the visual fx, this should account for the majority of behaviour that you might want to account for (can still have set behaviours, but deal with them in the visual fx class).
- [  ]  Add towers
- [  ]  Be able to place towers via player abilities

----
- [ x ]  There is a bug where if an enemy is dying and you hit them then they get back up
- [ x ]  Add a top/tailed cylinder and that fire effect with the gradient and you should get a nice shockwave type effect
- [ x ]  Rotate a mask of a texture and you should get the swipe you want