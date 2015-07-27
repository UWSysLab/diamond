from formless import annotate

isAdmin = annotate.Radio(["Yes", "No"])

class IResetRankForm(annotate.TypedInterface):
    
    def resetRank(self, ctx=annotate.Context()):
        pass
    resetRank = annotate.autocallable(resetRank)

class IStopServerForm(annotate.TypedInterface):
    
    def stopServer(self, ctx=annotate.Context()):
        pass
    stopServer = annotate.autocallable(stopServer)

class IDeleteUserForm(annotate.TypedInterface):
    
    def deleteUser(self, ctx=annotate.Context(), username=annotate.String(required=True)):
        pass
    deleteUser = annotate.autocallable(deleteUser, invisible=True)

class IKickUserForm(annotate.TypedInterface):
    
    def kickUser(self, ctx=annotate.Context(), username=annotate.String(required=True)):
        pass
    kickUser = annotate.autocallable(kickUser, invisible=True)

class INewGameForm(annotate.TypedInterface):
    
    def deleteGame(self, ctx=annotate.Context(), gameId=annotate.String(required=True)):
        pass
    deleteGame = annotate.autocallable(deleteGame, invisible=True)

class IEditUserForm(annotate.TypedInterface):
    
    def modifyUser(self, ctx=annotate.Context(), oldPassword=annotate.PasswordEntry(), password=annotate.Password(), isAdministrator=isAdmin):
        pass
    modifyUser = annotate.autocallable(modifyUser)

class IBulletinForm(annotate.TypedInterface):
    
    def addNewBulletin(self, ctx=annotate.Context(), message=annotate.String(required=True)): pass
    addNewBulletin = annotate.autocallable(addNewBulletin)
    
    def deleteBulletin(self, ctx=annotate.Context(), bulletinId=annotate.String(required=True)): pass
    deleteBulletin = annotate.autocallable(deleteBulletin, invisible=True)
