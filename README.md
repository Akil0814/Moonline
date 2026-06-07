# Moonline

## Known Issue / Follow-up
`SceneId` 目前仍包含具体业务场景名，因此 `engine/scene` 还不是完全业务无关。   
后续如需进一步通用化，可将 `SceneId` 抽象为通用 `SceneKey`。