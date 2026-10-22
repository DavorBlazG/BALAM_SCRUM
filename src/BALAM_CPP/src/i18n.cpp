#include "i18n.hpp"
#include <array>
#include <cstddef>

namespace i18n {
namespace {
using Row = std::array<const char*, 4>;
constexpr Row texts[] = {
    /* SelectLanguage  */ {"SELECCIONAR IDIOMA", "SELECCIONAR IDIOMA", "SELECT LANGUAGE", "VELG SPRÅK"},
    /* ChooseLanguage  */ {"Elige un idioma [1-4]: ", "Tria un idioma [1-4]: ", "Choose language [1-4]: ", "Velg språk [1-4]: "},
    /* InvalidOption   */ {"Opción no válida.", "Opció no vàlida.", "Invalid option.", "Ugyldig valg."},
    /* Welcome         */ {"MENÚ DE BIENVENIDA", "MENÚ DE BENVINGUDA", "WELCOME MENU", "VELKOMSTMENY"},
    /* Register        */ {"Crear cuenta", "Crear compte", "Create account", "Opprett konto"},
    /* Login           */ {"Iniciar sesión", "Iniciar sessió", "Sign in", "Logg inn"},
    /* ChangeLanguage  */ {"Cambiar idioma", "Canviar idioma", "Change language", "Bytt språk"},
    /* Exit            */ {"Salir", "Sortir", "Exit", "Avslutt"},
    /* ChooseOption    */ {"Selecciona una opción: ", "Selecciona una opció: ", "Choose an option: ", "Velg et alternativ: "},
    /* AccountExists   */ {"Ya existe una cuenta local. Esta versión admite solo una.", "Ja existeix un compte local. Aquesta versió només n'admet un.", "A local account already exists. This version supports only one.", "En lokal konto finnes allerede. Denne versjonen støtter bare én."},
    /* CreateAccount   */ {"CREAR CUENTA LOCAL", "CREAR COMPTE LOCAL", "CREATE LOCAL ACCOUNT", "OPPRETT LOKAL KONTO"},
    /* Email           */ {"Correo electrónico: ", "Correu electrònic: ", "Email: ", "E-post: "},
    /* AccountPassword */ {"Contraseña de cuenta: ", "Contrasenya del compte: ", "Account password: ", "Kontopassord: "},
    /* ConfirmPassword */ {"Repite la contraseña: ", "Repeteix la contrasenya: ", "Repeat password: ", "Gjenta passordet: "},
    /* InvalidEmail    */ {"Introduce un correo válido (ejemplo@dominio.com).", "Introdueix un correu vàlid (exemple@domini.com).", "Enter a valid email (example@domain.com).", "Skriv inn en gyldig e-post (navn@domene.no)."},
    /* MinPassword     */ {"La contraseña debe tener al menos 8 caracteres.", "La contrasenya ha de tenir com a mínim 8 caràcters.", "Password must contain at least 8 characters.", "Passordet må ha minst 8 tegn."},
    /* PasswordMismatch*/ {"Las contraseñas no coinciden.", "Les contrasenyes no coincideixen.", "Passwords do not match.", "Passordene er ikke like."},
    /* AccountCreated  */ {"Cuenta creada correctamente.", "Compte creat correctament.", "Account created successfully.", "Kontoen ble opprettet."},
    /* StorageError    */ {"No se pudieron guardar o leer los datos de autenticación.", "No s'han pogut desar o llegir les dades d'autenticació.", "Could not read or save authentication data.", "Kunne ikke lese eller lagre autentiseringsdata."},
    /* LoginTitle      */ {"INICIAR SESIÓN", "INICIAR SESSIÓ", "SIGN IN", "LOGG INN"},
    /* LoginFailed     */ {"Correo o contraseña incorrectos.", "Correu o contrasenya incorrectes.", "Incorrect email or password.", "Feil e-post eller passord."},
    /* LoginSuccess    */ {"Sesión iniciada.", "Sessió iniciada.", "Signed in.", "Innlogging vellykket."},
    /* AttemptsExhausted*/{"Demasiados intentos. Vuelve al menú principal.", "Massa intents. Torna al menú principal.", "Too many attempts. Returning to the main menu.", "For mange forsøk. Tilbake til hovedmenyen."},
    /* MasterCreate    */ {"CONFIGURAR CONTRASEÑA MAESTRA", "CONFIGURAR CONTRASENYA MESTRA", "SET UP MASTER PASSWORD", "OPPRETT HOVEDPASSORD"},
    /* MasterUnlock    */ {"DESBLOQUEAR BÓVEDA", "DESBLOQUEJAR CAIXA FORTA", "UNLOCK VAULT", "LÅS OPP HVELV"},
    /* MasterPassword  */ {"Contraseña maestra: ", "Contrasenya mestra: ", "Master password: ", "Hovedpassord: "},
    /* ConfirmMaster   */ {"Confirma la contraseña maestra: ", "Confirma la contrasenya mestra: ", "Confirm master password: ", "Bekreft hovedpassord: "},
    /* MasterCreated   */ {"Contraseña maestra configurada.", "Contrasenya mestra configurada.", "Master password configured.", "Hovedpassord er opprettet."},
    /* MasterWrong     */ {"Contraseña maestra incorrecta.", "Contrasenya mestra incorrecta.", "Incorrect master password.", "Feil hovedpassord."},
    /* VaultTitle      */ {"MI BÓVEDA", "LA MEVA CAIXA FORTA", "MY VAULT", "MITT HVELV"},
    /* VaultWarning    */ {"PROTOTIPO: entradas solo en memoria; se borran al bloquear o salir.", "PROTOTIP: entrades només en memòria; s'esborren en bloquejar o sortir.", "PROTOTYPE: entries exist only in memory; locking or exiting erases them.", "PROTOTYPE: Oppføringer lagres kun i minnet og slettes ved låsing eller avslutning."},
    /* AddEntry        */ {"Añadir contraseña", "Afegir contrasenya", "Add password", "Legg til passord"},
    /* ShowEntries     */ {"Ver entradas", "Veure entrades", "List entries", "Vis oppføringer"},
    /* SearchEntries   */ {"Buscar por servicio", "Cercar per servei", "Search by service", "Søk etter tjeneste"},
    /* EditEntry       */ {"Modificar entrada", "Modificar entrada", "Edit entry", "Rediger oppføring"},
    /* DeleteEntry     */ {"Eliminar entrada", "Eliminar entrada", "Delete entry", "Slett oppføring"},
    /* GeneratePassword*/ {"Generar contraseña segura", "Generar contrasenya segura", "Generate secure password", "Generer sikkert passord"},
    /* RevealEntry     */ {"Mostrar contraseña de una entrada", "Mostrar la contrasenya d'una entrada", "Reveal an entry's password", "Vis passordet til en oppføring"},
    /* LockVault       */ {"Bloquear bóveda", "Bloquejar caixa forta", "Lock vault", "Lås hvelvet"},
    /* Service         */ {"Servicio: ", "Servei: ", "Service: ", "Tjeneste: "},
    /* Username        */ {"Usuario o correo: ", "Usuari o correu: ", "Username or email: ", "Brukernavn eller e-post: "},
    /* EntryPassword   */ {"Contraseña del servicio: ", "Contrasenya del servei: ", "Service password: ", "Tjenestepassord: "},
    /* EntryAdded      */ {"Entrada añadida (solo en memoria).", "Entrada afegida (només en memòria).", "Entry added (in memory only).", "Oppføring lagt til (bare i minnet)."},
    /* EntriesEmpty    */ {"No hay entradas guardadas en esta sesión.", "No hi ha entrades desades en aquesta sessió.", "No entries saved in this session.", "Ingen oppføringer lagret i denne økten."},
    /* EntryNumber     */ {"Número de entrada: ", "Número d'entrada: ", "Entry number: ", "Oppføringsnummer: "},
    /* EntryMissing    */ {"No se encontró esa entrada.", "No s'ha trobat aquesta entrada.", "Entry not found.", "Fant ikke oppføringen."},
    /* EntryUpdated    */ {"Entrada modificada (solo en memoria).", "Entrada modificada (només en memòria).", "Entry updated (in memory only).", "Oppføring endret (bare i minnet)."},
    /* EntryDeleted    */ {"Entrada eliminada.", "Entrada eliminada.", "Entry deleted.", "Oppføring slettet."},
    /* ConfirmDelete   */ {"¿Eliminar esta entrada? (s/n): ", "Vols eliminar aquesta entrada? (s/n): ", "Delete this entry? (y/n): ", "Slette denne oppføringen? (j/n): "},
    /* Cancelled       */ {"Operación cancelada.", "Operació cancel·lada.", "Operation cancelled.", "Handlingen ble avbrutt."},
    /* SearchQuery     */ {"Nombre del servicio: ", "Nom del servei: ", "Service name: ", "Tjenestenavn: "},
    /* NoResults       */ {"No hay coincidencias.", "No hi ha coincidències.", "No matches.", "Ingen treff."},
    /* PasswordLength  */ {"Longitud de contraseña (8-64): ", "Longitud de la contrasenya (8-64): ", "Password length (8-64): ", "Passordlengde (8-64): "},
    /* GeneratedPassword*/{"Contraseña generada: ", "Contrasenya generada: ", "Generated password: ", "Generert passord: "},
    /* InvalidLength   */ {"Longitud incorrecta.", "Longitud incorrecta.", "Invalid length.", "Ugyldig lengde."},
    /* NoRealSecrets   */ {"Usa solo datos ficticios: todavía no hay almacenamiento cifrado.", "Fes servir només dades fictícies: encara no hi ha emmagatzematge xifrat.", "Use dummy data only: encrypted storage is not implemented yet.", "Bruk kun testdata: kryptert lagring er ennå ikke implementert."},
    /* ExitThanks      */ {"Hasta pronto. BALAM cerrado.", "Fins aviat. BALAM tancat.", "Goodbye. BALAM closed.", "Ha det. BALAM er avsluttet."},
    /* LanguageChanged */ {"Idioma actualizado.", "Idioma actualitzat.", "Language updated.", "Språket er endret."},
    /* GeneralError    */ {"Se ha producido un error: ", "S'ha produït un error: ", "An error occurred: ", "En feil oppstod: "},
    /* AskAgain        */ {"Inténtalo de nuevo.", "Torna-ho a provar.", "Please try again.", "Prøv igjen."},
    /* ShowPassword    */ {"Contraseña: ", "Contrasenya: ", "Password: ", "Passord: "},
    /* MinPasswordInfo */ {"Mínimo 8 caracteres. No uses contraseñas reales en este prototipo.", "Mínim 8 caràcters. No facis servir contrasenyes reals en aquest prototip.", "Minimum 8 characters. Do not use real passwords in this prototype.", "Minst 8 tegn. Ikke bruk ekte passord i denne prototypen."}
};
}
std::string_view text(Language language, Message key)
{
    return texts[static_cast<std::size_t>(key)][static_cast<std::size_t>(language)];
}
const char* languageName(Language language)
{
    switch (language) {
        case Language::Spanish: return "Español";
        case Language::Catalan: return "Català";
        case Language::English: return "English";
        default: return "Norsk bokmål";
    }
}
}
