import './DialogueBox.css';

export const DialogueBox = ({ speaker, text, active, isGamepad = true }) => {
  if (!active) return null;

  return (
    // Usamos la combinación de speaker y texto como KEY para forzar 
    // a React a reiniciar las animaciones (como el efecto de máquina de escribir)
    <div className="dialogue-container" key={`${speaker.name}-${text}`}>
      <div className="dialogue-portrait-wrapper">
        <img 
          src={`enemies/${speaker.picture.toLowerCase()}.png`} 
          className="dialogue-portrait" 
          alt={speaker.name} 
        />
      </div>
      
      <div className="dialogue-content">
        <div className="dialogue-speaker">{speaker.name}</div>
        <div className="dialogue-text">{text}</div>
      </div>

      {/* Indicador de "Siguiente" dinámico */}
      <div className="dialogue-next-indicator">
        {isGamepad ? (
          <div className="btn-a">A</div>
        ) : (
          <div className="btn-enter">ENTER</div>
        )}
      </div>

    </div>
  );
};