import { HealthBar } from '../../HealthBar/HealthBar';

export const EnemyHud = ({ picture, title, hp }) => {
  return (
    <div className="character-hud enemy">
      <div className="unit-display enemy">
        <img src={picture} className="portrait enemy" alt={title} />
        <div className="bars-container">
          <span className="bars-title enemy">{title}</span>
          <HealthBar hp={hp} type="sub" />
        </div>
      </div>
    </div>
  );
}